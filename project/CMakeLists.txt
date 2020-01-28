cmake_minimum_required(VERSION 3.10)

# set the project name
project(Warble)

add_compile_options(-std=c++11)

# add the executable
add_executable(Warble test_file.cxx)