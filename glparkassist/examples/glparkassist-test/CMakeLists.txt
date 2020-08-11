
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 0. CMake Defs

# 1. Project Name
project(glparkassist-test)

# 2. Project Build
set(BIN_NAME "glparkassist-test")
file(GLOB SRC_FILES "*.h" "*.cpp" "../*.h" "../*.cpp")
add_executable(${BIN_NAME} ${SRC_FILES})
set_target_properties(${BIN_NAME} PROPERTIES OUTPUT_NAME glparkassist-test)
target_link_libraries(${BIN_NAME} libparkassist)

# 3. Project Install
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

