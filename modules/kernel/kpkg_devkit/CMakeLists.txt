cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_custom_target(build_kpkg_devkit ALL COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && make
        CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(clean_kpkg_devkit COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && make clean)