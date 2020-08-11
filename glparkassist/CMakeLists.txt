
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 0. CMake Defs

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

cmake_policy(VERSION 2.6)

# 1. Project Name
project(glparkassist)

# 2. Env (Include/Lib Path, C/CXX/LD FLAGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/dist)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)

include_directories(include)
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/include")
#link_directories("${CMAKE_SOURCE_DIR}/3rdparty/lib")

add_definitions("-fPIC -Wno-deprecated -Wno-deprecated-declarations") #-W -Wall

# 3. Sub Projects
add_subdirectory(3rdparty)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(examples)

# 5. Project Install
# N/A

