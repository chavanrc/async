cmake_minimum_required(VERSION 3.20)
project(Examples)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS thread REQUIRED)

find_package(Threads REQUIRED)

add_executable(Examples main.cpp)

target_link_libraries(Examples Threads::Threads Boost::thread)
