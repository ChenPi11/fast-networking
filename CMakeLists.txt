cmake_minimum_required(VERSION 3.10)
project(fast-networking)
set(CMAKE_C_STANDARD 99)
include_directories(include)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Wildcard source files recursively.
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.cpp")
add_executable(fast-networking ${SOURCES})
