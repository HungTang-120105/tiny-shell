cmake_minimum_required(VERSION 3.15)

project(tiny_shell C CXX)

include_directories(include)

file(GLOB SOURCES "src/*.c" "src/*.cpp")

add_executable(myShell ${SOURCES})
