cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

project(FoxMarkov)

add_compile_options(-Wall -Wextra -pedantic -Wno-writable-strings -Wno-unused-command-line-argument)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

