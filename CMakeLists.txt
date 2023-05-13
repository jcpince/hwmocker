cmake_minimum_required(VERSION 3.22)

project(hwmocker)

add_compile_options(-include ${CMAKE_BINARY_DIR}/generated/hwmocker/config.h)

include_directories(include ${CMAKE_SOURCE_DIR}/include/C-API
                    ${CMAKE_BINARY_DIR}/generated)

include(cmake/config.cmake)
include(cmake/helpers.cmake)

add_subdirectory(src)
add_subdirectory_ifdef(CONFIG_HWMOCK_TESTS tests)

configure_file(include/C-API/hwmocker/config.h.in generated/hwmocker/config.h)
