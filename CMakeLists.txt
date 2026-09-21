cmake_minimum_required(VERSION 3.14)
project(json2ts CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(json2ts src/main.cpp)
target_include_directories(json2ts PRIVATE include)
