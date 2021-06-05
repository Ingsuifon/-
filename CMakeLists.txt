cmake_minimum_required(VERSION 3.19)
project(FileSystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(FileSystem main.cpp FileSystem.cpp FileSystem.h common.h)