cmake_minimum_required(VERSION 3.15)

project(tiny_shell C CXX)

include_directories(include)

file(GLOB HEADERS "src/include/*.h")
file(GLOB SOURCES "src/process/*.c" "src/process/*.cpp" "src/main.cpp")

add_executable(myShell ${SOURCES})

target_link_libraries(myShell wbemuuid ole32 oleaut32)