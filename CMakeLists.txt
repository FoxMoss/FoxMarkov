cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

project(FoxMarkov)

add_compile_options(-Wall -Wextra -pedantic -Wno-writable-strings -Wno-unused-command-line-argument)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CPACK_PACKAGE_CONTACT foxmoss@mediaology.com)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER FoxMoss)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_PACKAGE_NAME "foxkov")
set(CPACK_PACKAGE_VENDOR "foxmoss.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "foxkov - Lingustic Markov chain toolset.")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

add_subdirectory(src)

