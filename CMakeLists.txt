cmake_minimum_required(VERSION 3.28)
project(rock_paper_scissors_lizard_spock)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/core)
add_subdirectory(src/server)
add_subdirectory(src/client)
