cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(performance)