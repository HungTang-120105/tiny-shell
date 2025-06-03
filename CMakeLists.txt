cmake_minimum_required(VERSION 3.15)

project(tiny_shell C CXX)

include_directories(src/include)

file(GLOB HEADERS "src/include/*.h")
file(GLOB SOURCES "src/process/*.c" "src/process/*.cpp" "src/main.cpp" "src/snake_game.cpp" "src/calculator.cpp" "src/system_utils.cpp")

add_definitions(-D_WIN32_WINNT=0x0600)
add_executable(myShell ${SOURCES})

target_link_libraries(myShell kernel32 wbemuuid ole32 oleaut32)

set(CMAKE_CXX_STANDARD 17)

# Force static link for GCC/MinGW
if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()