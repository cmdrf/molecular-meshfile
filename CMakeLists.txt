cmake_minimum_required(VERSION 3.3)
project(molecular-meshfile)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(3rdparty)
add_subdirectory(compiler)

add_library(molecular-meshfile INTERFACE)
target_include_directories(molecular-meshfile INTERFACE .)
add_library(molecular::meshfile ALIAS molecular-meshfile)
