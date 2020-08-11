
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 1. Project Name
project(src)

# 2. Project Env (Include/Lib Path, C/CXX/LD FLAGS)

# 3. Project Build
set(LIB_NAME "libparkassist")
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/include/*.h" "*.h" "*.cpp")

add_library(${LIB_NAME} SHARED ${SRC_FILES})
set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME glparkassist)
target_link_libraries(${LIB_NAME} EGL GLESv2 drm gbm)

# 4. Project Install
#message (${CMAKE_SOURCE_DIR}/build)
install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

