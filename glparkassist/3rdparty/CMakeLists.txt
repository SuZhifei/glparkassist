
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

project(3rdparty)

# 2. Env (Include/Lib Path, C/CXX/LD FLAGS)

# 4. Sub Proj
add_subdirectory(include)
#add_subdirectory(lib)

# 5. Project Install
# N/A

