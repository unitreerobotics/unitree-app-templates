cmake_minimum_required(VERSION 3.5)
project(unitree_sdk2_example VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)
find_package(unitree_sdk2 REQUIRED)

add_executable(cmd src/cmd.cpp)
target_link_libraries(cmd PRIVATE unitree_sdk2)
