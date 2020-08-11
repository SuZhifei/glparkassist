
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 0. CMake Defs

# 1. Project Name
project(lib)

# 2. Env (Include/Lib Path, C/CXX/LD FLAGS)

# 3. Sub Projects

# 4. Project Install
file(GLOB LIBS ${CMAKE_CURRENT_SOURCE_DIR}/*.so*)
install(FILES ${LIBS} DESTINATION ${CMAKE_INSTALL_LIBDIR})

