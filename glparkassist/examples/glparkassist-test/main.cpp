/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 * Copyright (c) 2013 Anand Balagopalakrishnan <anandb@ti.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Based on a egl cube test app originally written by Arvin Schnell */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "gbm/gbm.h"

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "render.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MAX_DISPLAYS 	(4)
uint8_t DISP_ID = 0;
uint8_t all_display = 0;
int8_t connector_id = -1;

Render* mRenderPtr;//Render对象需要在EGL初始化完成后再构建。

static struct {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
} gl;

static struct {
	struct gbm_device *dev;
	struct gbm_surface *surface;
} gbm;

static struct {
	int fd;
	uint32_t ndisp;
	uint32_t crtc_id[MAX_DISPLAYS];
	uint32_t connector_id[MAX_DISPLAYS];
	uint32_t resource_id;
	uint32_t encoder[MAX_DISPLAYS];
	drmModeModeInfo *mode[MAX_DISPLAYS];
	drmModeConnector *connectors[MAX_DISPLAYS];
} drm;

struct drm_fb {
	struct gbm_bo *bo;
	uint32_t fb_id;
};

static int init_drm(void)
{
	static const char *modules[] = {
			"omapdrm", "i915", "radeon", "nouveau", "vmwgfx", "exynos"
	};
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	int i, j;
	uint32_t maxRes, curRes;

	for (i = 0; i < ARRAY_SIZE(modules); i++) {
		printf("trying to load module %s...", modules[i]);
		drm.fd = drmOpen(modules[i], NULL);
		if (drm.fd < 0) {
			printf("failed.\n");
		} else {
			printf("success.\n");
			break;
		}
	}

	if (drm.fd < 0) {
		printf("could not open drm device\n");
		return -1;
	}

	resources = drmModeGetResources(drm.fd);
	if (!resources) {
		printf("drmModeGetResources failed: %s\n", strerror(errno));
		return -1;
	}
	drm.resource_id = (uint32_t) resources;

	/* find a connected connector: */
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm.fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED) {
			/* choose the first supported mode */
			drm.mode[drm.ndisp] = &connector->modes[0];
			drm.connector_id[drm.ndisp] = connector->connector_id;

			for (j=0; j<resources->count_encoders; j++) {
				encoder = drmModeGetEncoder(drm.fd, resources->encoders[j]);
				if (encoder->encoder_id == connector->encoder_id)
					break;

				drmModeFreeEncoder(encoder);
				encoder = NULL;
			}

			if (!encoder) {
				printf("no encoder!\n");
				return -1;
			}

			drm.encoder[drm.ndisp]  = (uint32_t) encoder;
			drm.crtc_id[drm.ndisp] = encoder->crtc_id;
			drm.connectors[drm.ndisp] = connector;

			printf("### Display [%d]: CRTC = %d, Connector = %d\n", drm.ndisp, drm.crtc_id[drm.ndisp], drm.connector_id[drm.ndisp]);
			printf("\tMode chosen [%s] : Clock => %d, Vertical refresh => %d, Type => %d\n", drm.mode[drm.ndisp]->name, drm.mode[drm.ndisp]->clock, drm.mode[drm.ndisp]->vrefresh, drm.mode[drm.ndisp]->type);
			printf("\tHorizontal => %d, %d, %d, %d, %d\n", drm.mode[drm.ndisp]->hdisplay, drm.mode[drm.ndisp]->hsync_start, drm.mode[drm.ndisp]->hsync_end, drm.mode[drm.ndisp]->htotal, drm.mode[drm.ndisp]->hskew);
			printf("\tVertical => %d, %d, %d, %d, %d\n", drm.mode[drm.ndisp]->vdisplay, drm.mode[drm.ndisp]->vsync_start, drm.mode[drm.ndisp]->vsync_end, drm.mode[drm.ndisp]->vtotal, drm.mode[drm.ndisp]->vscan);

			/* If a connector_id is specified, use the corresponding display */
			if ((connector_id != -1) && (connector_id == drm.connector_id[drm.ndisp]))
				DISP_ID = drm.ndisp;

			/* If all displays are enabled, choose the connector with maximum
			* resolution as the primary display */
			if (all_display) {
				maxRes = drm.mode[DISP_ID]->vdisplay * drm.mode[DISP_ID]->hdisplay;
				curRes = drm.mode[drm.ndisp]->vdisplay * drm.mode[drm.ndisp]->hdisplay;

				if (curRes > maxRes)
					DISP_ID = drm.ndisp;
			}

			drm.ndisp++;
		} else {
			drmModeFreeConnector(connector);
		}
	}

	if (drm.ndisp == 0) {
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector..
		 */
		printf("no connected connector!\n");
		return -1;
	}

	return 0;
}

static int init_gbm(void)
{
	gbm.dev = gbm_create_device(drm.fd);

	gbm.surface = gbm_surface_create(gbm.dev,
			drm.mode[DISP_ID]->hdisplay, drm.mode[DISP_ID]->vdisplay,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if (!gbm.surface) {
		printf("failed to create gbm surface\n");
		return -1;
	}

	return 0;
}

static int init_egl(void)
{
	EGLint major, minor, n;

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_SAMPLES, 4,
		EGL_NONE
	};

	gl.display = eglGetDisplay((EGLNativeDisplayType)gbm.dev);

	if (!eglInitialize(gl.display, &major, &minor)) {
		printf("failed to initialize\n");
		return -1;
	}

	printf("Using display %p with EGL version %d.%d\n",
			gl.display, major, minor);

	printf("EGL Version \"%s\"\n", eglQueryString(gl.display, EGL_VERSION));
	printf("EGL Vendor \"%s\"\n", eglQueryString(gl.display, EGL_VENDOR));
	printf("EGL Extensions \"%s\"\n", eglQueryString(gl.display, EGL_EXTENSIONS));

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("failed to bind api EGL_OPENGL_ES_API\n");
		return -1;
	}

	if (!eglChooseConfig(gl.display, config_attribs, &gl.config, 1, &n) || n != 1) {
		printf("failed to choose config: %d\n", n);
		return -1;
	}

	gl.context = eglCreateContext(gl.display, gl.config,
			EGL_NO_CONTEXT, context_attribs);
	if (gl.context == NULL) {
		printf("failed to create context\n");
		return -1;
	}

	gl.surface = eglCreateWindowSurface(gl.display, gl.config, gbm.surface, NULL);
	if (gl.surface == EGL_NO_SURFACE) {
		printf("failed to create egl surface\n");
		return -1;
	}

	/* connect the context to the surface */
	eglMakeCurrent(gl.display, gl.surface, gl.surface, gl.context);

	return 0;
}

static void exit_gbm(void)
{
        gbm_surface_destroy(gbm.surface);
        gbm_device_destroy(gbm.dev);
        return;
}

static void exit_egl(void)
{
	delete mRenderPtr;
	eglDestroySurface(gl.display, gl.surface);
	eglDestroyContext(gl.display, gl.context);
	eglTerminate(gl.display);
	return;
}

static void exit_drm(void)
{

        drmModeRes *resources;
        int i;

        resources = (drmModeRes *)drm.resource_id;
        for (i = 0; i < resources->count_connectors; i++) {
                drmModeFreeEncoder((drmModeEncoderPtr)drm.encoder[i]);
                drmModeFreeConnector(drm.connectors[i]);
        }
        drmModeFreeResources((drmModeResPtr)drm.resource_id);
        drmClose(drm.fd);
        return;
}

void cleanup_kmscube(void)
{
	exit_egl();
	exit_gbm();
	exit_drm();
	printf("Cleanup of GL, GBM and DRM completed\n");
	return;
}

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	struct drm_fb *fb = (drm_fb *)data;
	struct gbm_device *gbm = gbm_bo_get_device(bo);

	if (fb->fb_id)
		drmModeRmFB(drm.fd, fb->fb_id);

	free(fb);
}

static struct drm_fb * drm_fb_get_from_bo(struct gbm_bo *bo)
{
	struct drm_fb *fb = (drm_fb*)gbm_bo_get_user_data(bo);
	uint32_t width, height, stride, handle;
	int ret;

	if (fb)
		return fb;

	fb = (drm_fb*)calloc(1, sizeof *fb);
	fb->bo = bo;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	stride = gbm_bo_get_stride(bo);
	handle = gbm_bo_get_handle(bo).u32;

	ret = drmModeAddFB(drm.fd, width, height, 24, 32, stride, handle, &fb->fb_id);
	if (ret) {
		printf("failed to create fb: %s\n", strerror(errno));
		free(fb);
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

static void vblank_handler(int fd, unsigned int frame,
		  unsigned int sec, unsigned int usec, void *data)
{
	//nothing to do?
}

static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	int *waiting_for_flip = (int*)data;
	*waiting_for_flip = 0;
}

void print_usage()
{
	printf("Usage : kmscube <options>\n");
	printf("\t-h : Help\n");
	printf("\t-a : Enable all displays\n");
	printf("\t-c <id> : Display using connector_id [if not specified, use the first connected connector]\n");
	printf("\t-n <number> (optional): Number of frames to render\n");
}

int kms_signalhandler(int signum)
{
	switch(signum) {
	case SIGINT:
        case SIGTERM:
                /* Allow the pending page flip requests to be completed before
                 * the teardown sequence */
                sleep(1);
                printf("Handling signal number = %d\n", signum);
		cleanup_kmscube();
		break;
	default:
		printf("Unknown signal\n");
		break;
	}
	exit(1);
}

int main(int argc, char *argv[])
{
	fd_set fds;
	drmEventContext evctx = {
			.version = DRM_EVENT_CONTEXT_VERSION,
			.vblank_handler = vblank_handler,
			.page_flip_handler = page_flip_handler,
	};
	struct gbm_bo *bo;
	struct drm_fb *fb;
	uint32_t i = 0;
	int ret;
	int opt;
	int frame_count = -1;

	signal(SIGINT, (__sighandler_t)kms_signalhandler);
	signal(SIGTERM, (__sighandler_t)kms_signalhandler);

	while ((opt = getopt(argc, argv, "ahc:n:")) != -1) {
		switch(opt) {
		case 'a':
			all_display = 1;
			break;

		case 'h':
			print_usage();
			return 0;

		case 'c':
			connector_id = atoi(optarg);
			break;
		case 'n':
			frame_count = atoi(optarg);
			break;


		default:
			printf("Undefined option %s\n", argv[optind]);
			print_usage();
			return -1;
		}
	}

	if (all_display) {
		printf("### Enabling all displays\n");
		connector_id = -1;
	}

	ret = init_drm();
	if (ret) {
		printf("failed to initialize DRM\n");
		return ret;
	}
	printf("### Primary display => ConnectorId = %d, Resolution = %dx%d\n",
			drm.connector_id[DISP_ID], drm.mode[DISP_ID]->hdisplay,
			drm.mode[DISP_ID]->vdisplay);

	FD_ZERO(&fds);
	FD_SET(drm.fd, &fds);

	ret = init_gbm();
	if (ret) {
		printf("failed to initialize GBM\n");
		return ret;
	}

	ret = init_egl();
	if (ret) {
		printf("failed to initialize EGL\n");
		return ret;
	}

	mRenderPtr = new Render();

	eglSwapBuffers(gl.display, gl.surface);
	bo = gbm_surface_lock_front_buffer(gbm.surface);
	fb = drm_fb_get_from_bo(bo);

	/* set mode: */
	if (all_display) {
		for (i=0; i<drm.ndisp; i++) {
			ret = drmModeSetCrtc(drm.fd, drm.crtc_id[i], fb->fb_id, 0, 0,
					&drm.connector_id[i], 1, drm.mode[i]);
			if (ret) {
				printf("display %d failed to set mode: %s\n", i, strerror(errno));
				return ret;
			}
		}
	} else {
		ret = drmModeSetCrtc(drm.fd, drm.crtc_id[DISP_ID], fb->fb_id,
				0, 0, &drm.connector_id[DISP_ID], 1, drm.mode[DISP_ID]);
		if (ret) {
			printf("display %d failed to set mode: %s\n", DISP_ID, strerror(errno));
			return ret;
		}
	}

	while (true) {
		struct gbm_bo *next_bo;
		int waiting_for_flip = 1;

		mRenderPtr->draw();

		eglSwapBuffers(gl.display, gl.surface);
		next_bo = gbm_surface_lock_front_buffer(gbm.surface);
		fb = drm_fb_get_from_bo(next_bo);

		/*
		 * Here you could also update drm plane layers if you want
		 * hw composition
		 */

		ret = drmModePageFlip(drm.fd, drm.crtc_id[DISP_ID], fb->fb_id,
				DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
		if (ret) {
			printf("failed to queue page flip: %s\n", strerror(errno));
			return -1;
		}

		while (waiting_for_flip) {
			ret = select(drm.fd + 1, &fds, NULL, NULL, NULL);
			if (ret < 0) {
				printf("select err: %s\n", strerror(errno));
				return ret;
			} else if (ret == 0) {
				printf("select timeout!\n");
				return -1;
			} else if (FD_ISSET(0, &fds)) {
				continue;
			}
			drmHandleEvent(drm.fd, &evctx);
		}

		/* release last buffer to render on again: */
		gbm_surface_release_buffer(gbm.surface, bo);
		bo = next_bo;
	}

	cleanup_kmscube();
	printf("\n Exiting kmscube \n");

	return ret;
}
