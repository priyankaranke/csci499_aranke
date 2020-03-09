cmake_minimum_required(VERSION 3.10)

project(myProject)
add_compile_options(-std=c++17)

cmake_policy(SET CMP0060 NEW)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)