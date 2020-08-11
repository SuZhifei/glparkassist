#include <stdlib.h>
#include "GLES2/gl2.h"
#include "render.h"

static struct {
	GLuint program;
	GLuint vbo;
	GLuint positionsoffset;
	GLuint vertex_shader, fragment_shader;
} gl;

Render::Render(){
	initGL();
}

Render::~Render(){
	exitGL();
}

void Render::initGL(){
	GLint ret;

	static const GLfloat vVertices[] = {
			// front
        -0.5f, -0.5f, 0.0f, // left  
        +0.5f, -0.5f, 0.0f, // right 
        0.0f,  +0.5f, 0.0f  // top 
	};

	static const char *vertex_shader_source =
			"attribute vec3 aPos;        \n"
			"\n"
			"void main()                        \n"
			"{                                  \n"
			"    gl_Position = vec4(aPos, 1.0);\n"
			"}                                  \n";

	static const char *fragment_shader_source =
			"precision mediump float;           \n"
			"                                   \n"
			"void main()                        \n"
			"{                                  \n"
			"    gl_FragColor = vec4(1.0, 0.5, 0.2, 0.3);  \n"
			"}                                  \n";

	gl.vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(gl.vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(gl.vertex_shader);

	glGetShaderiv(gl.vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("vertex shader compilation failed!:\n");
		glGetShaderiv(gl.vertex_shader, GL_INFO_LOG_LENGTH, &ret);
		if (ret > 1) {
			log = (char*)malloc(ret);
			glGetShaderInfoLog(gl.vertex_shader, ret, NULL, log);
			printf("%s", log);
		}

		return;
	}

	gl.fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(gl.fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(gl.fragment_shader);

	glGetShaderiv(gl.fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("fragment shader compilation failed!:\n");
		glGetShaderiv(gl.fragment_shader, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = (char*)malloc(ret);
			glGetShaderInfoLog(gl.fragment_shader, ret, NULL, log);
			printf("%s", log);
		}

		return;
	}

	gl.program = glCreateProgram();

	glAttachShader(gl.program, gl.vertex_shader);
	glAttachShader(gl.program, gl.fragment_shader);

	glBindAttribLocation(gl.program, 0, "aPos");

	glLinkProgram(gl.program);

	glGetProgramiv(gl.program, GL_LINK_STATUS, &ret);
	if (!ret) {
		char *log;

		printf("program linking failed!:\n");
		glGetProgramiv(gl.program, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1) {
			log = (char*)malloc(ret);
			glGetProgramInfoLog(gl.program, ret, NULL, log);
			printf("%s", log);
		}

		return;
	}

	glUseProgram(gl.program);

	//glViewport(0, 0, 800, 720);
	glEnable(GL_CULL_FACE);

	gl.positionsoffset = 0;

	glGenBuffers(1, &gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, gl.positionsoffset, sizeof(vVertices), &vVertices[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)gl.positionsoffset);
	glEnableVertexAttribArray(0);

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Render::exitGL()
{
	glDeleteProgram(gl.program);
	glDeleteBuffers(1, &gl.vbo);
	glDeleteShader(gl.fragment_shader);
	glDeleteShader(gl.vertex_shader);
	return;
}

void Render::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

