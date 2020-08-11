
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 1. Project Name
project(include)

# 2. Project Env (Include/Lib Path, C/CXX/LD FLAGS)

# 3. Project Build

# 4. Project Install
file(GLOB INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${INCLUDE} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

