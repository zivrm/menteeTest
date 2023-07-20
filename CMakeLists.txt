cmake_minimum_required(VERSION 3.4...3.18)
project(TEST_CLEANBUILD)
# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Find pybind11 package
find_package(pybind11 REQUIRED)
add_subdirectory(pybind11)
add_executable(my_executable main.cpp)
#pybind11_add_module(menteeRobTest src/main.cpp)
target_link_libraries(my_executable PRIVATE pybind11::embed)
# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.

#target_compile_definitions(menteeRobTest
#                           PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
