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
    qfsgen
    VERSION ${SW_VERSION}
    DESCRIPTION "QTools qfsgen - https://www.state-machine.com/qtools/qfsgen.html"
    LANGUAGES C
)

message(STATUS "Building 'qfsgen' for '${CMAKE_SYSTEM_NAME}'")

if(UNIX OR MSYS OR CMAKE_SYSTEM_NAME STREQUAL Linux OR CMAKE_SYSTEM_NAME STREQUAL MSYS OR CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(FS_DIR win32 posix)
else()
    set(FS_DIR win32)
endif()

add_executable(qfsgen
    source/main.c
    ${FS_DIR}/filesearch.c
)
target_include_directories(qfsgen PRIVATE include)
