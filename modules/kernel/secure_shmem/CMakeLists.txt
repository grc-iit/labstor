cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(KERNEL_SERVER_PATH ${CMAKE_SOURCE_DIR}/src/kernel_server)
add_custom_target(build-km ALL COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel && make KERNEL_SERVER=${KERNEL_SERVER_PATH} SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
#add_library(shmem_client client/shmem_user.cpp)