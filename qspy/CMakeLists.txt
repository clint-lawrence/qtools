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
    qspy
    VERSION ${SW_VERSION}
    DESCRIPTION "QTools qspy - https://www.state-machine.com/qtools/qspy.html"
    LANGUAGES C
)

message(STATUS "Building 'qspy' for '${CMAKE_SYSTEM_NAME}'")

if(UNIX OR MSYS OR CMAKE_SYSTEM_NAME STREQUAL Linux OR CMAKE_SYSTEM_NAME STREQUAL MSYS OR CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(PAL_DIR posix)
else()
    set(PAL_DIR win32)
endif()

add_executable(qspy
    source/qspy.c
    source/qspy_main.c
    source/qspy_be.c
    source/qspy_dict.c
    source/qspy_seq.c
    source/qspy_tx.c
    source/getopt.c
    ${PAL_DIR}/qspy_pal.c
)

target_include_directories(qspy PRIVATE
    ${PAL_DIR}
    include
)
target_compile_definitions(qspy PRIVATE QSPY_APP)
target_link_libraries(qspy PRIVATE $<$<PLATFORM_ID:Windows>:ws2_32>)