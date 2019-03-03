cmake_minimum_required(VERSION 3.10)
project(actor)

set(CMAKE_CXX_STANDARD 17)
find_package (Threads)

add_executable(actor main.cpp)
target_include_directories(actor PRIVATE include)
target_link_libraries (actor ${CMAKE_THREAD_LIBS_INIT})

add_executable(pingpong sample/pingpong/pingpong.cpp)
target_include_directories(pingpong PRIVATE include)
target_link_libraries (pingpong ${CMAKE_THREAD_LIBS_INIT})
