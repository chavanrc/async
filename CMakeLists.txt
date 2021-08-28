cmake_minimum_required(VERSION 3.20)
project(AsynExamples)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS thread REQUIRED)

find_package(Threads REQUIRED)

add_executable(AsynExamples main.cpp)

target_link_libraries(AsynExamples Threads::Threads Boost::thread)
