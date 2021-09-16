cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(../generic_posix/include)

add_library(labstor_fs_server server/labstor_fs_server.cpp)
add_library(labstor_fs_client client/labstor_fs_client.cpp)