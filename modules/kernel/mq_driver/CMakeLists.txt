cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

#SHMEM KM
add_custom_target(build-km ALL COMMAND cd server && make KPKG_DEVKIT=${KPKG_DEVKIT})

