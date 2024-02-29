# use a recent CMake version
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
cmake_policy(VERSION 3.23)
cmake_policy(SET CMP0083 NEW)

# first of all protect against in-source builds
file(REAL_PATH "${CMAKE_SOURCE_DIR}" _srcdir)
file(REAL_PATH "${CMAKE_BINARY_DIR}" _bindir)

if(${_srcdir} STREQUAL ${_bindir})
    message(FATAL_ERROR "  FATAL: In-source builds are not allowed!
        You should create a separate directory for build files.")
endif()
unset(_srcdir)
unset(_bindir)

# update CMAKE_MODULE_PATH
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/source/cmake ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR} ${HOME_PATH}/cmake)

if(NOT SW_VERSION)
    set(SW_VERSION "7.3.3" CACHE STRING "Software Version")
endif()

project(
    qtools
    VERSION ${SW_VERSION}
    DESCRIPTION "QTools - https://www.state-machine.com/qtools/"
    LANGUAGES C
)

add_subdirectory(qclean)
add_subdirectory(qfsgen)
add_subdirectory(qspy)
