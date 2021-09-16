cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(KPKG_DEVKIT ${CMAKE_SOURCE_DIR}/src/kernel_servver/kpkg_devkit)

#SHMEM KM
add_custom_target(build-km ALL COMMAND cd server && make KPKG_DEVKIT=${KPKG_DEVKIT})

