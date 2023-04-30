cmake_minimum_required(VERSION 3.22)

project(hwmocker)

include_directories(include
    ${CMAKE_BINARY_DIR})

include(cmake/config.cmake)
include(cmake/helpers.cmake)

add_subdirectory(src)
add_subdirectory_ifdef(CONFIG_HWMOCK_TESTS tests)

configure_file(include/hwmocker/config.h.in hwmocker/config.h)