cmake_minimum_required(VERSION 3.14)

project(json2ts
    VERSION 0.1.0
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/mitsuki31/json2ts
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(json2ts ${SOURCES})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/json2ts/version.in.h"
    "${CMAKE_CURRENT_BINARY_DIR}/include/json2ts/version.h"
    @ONLY
)

target_include_directories(json2ts PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/include"   # generated header
    "${CMAKE_CURRENT_SOURCE_DIR}/include"   # hand-written headers
)
