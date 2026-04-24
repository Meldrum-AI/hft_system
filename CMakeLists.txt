cmake_minimum_required(VERSION 3.20)
project(hft_system)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -DNDEBUG -flto -Wno-unused-result")

include_directories(.)
add_compile_definitions(TDSYS_PARAM)

add_executable(hft
    Main.cpp
)