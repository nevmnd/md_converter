cmake_minimum_required(VERSION 3.28)

project(md_converter VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message("Generated with config types: ${CMAKE_CFG_INTDIR}")

# set(CMAKE_VERBOSE_MAKEFILE TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Build type is set to: ${CMAKE_BUILD_TYPE}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/build/$<CONFIG>")

set(SOURCES main.cpp)

# set(BUILD_MD2HTML_EXECUTABLE OFF CACHE BOOL "Disable md2html executable build" FORCE)

add_subdirectory(md4c EXCLUDE_FROM_ALL)

add_executable(md_converter ${SOURCES})

target_link_libraries(md_converter md4c)

include_directories("${PROJECT_SOURCE_DIR}/md4c/src")

if (MSVC)
    add_compile_options(/W4 /WX) # Enable warnings and treat as errors
else()
    target_compile_options(md_converter -Wall -Wextra -pedantic)
endif()

message(STATUS "Files will be installed to: ${CMAKE_INSTALL_PREFIX}")

install(
    TARGETS md_converter
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
)