cmake_minimum_required(VERSION 3.10)
project(actor)

set(CMAKE_CXX_STANDARD 17)

add_executable(actor main.cpp)
target_include_directories(actor PRIVATE include)
