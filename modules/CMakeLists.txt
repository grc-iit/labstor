cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include kernel io_systems)

######MODULES
add_subdirectory(kernel/secure_shmem)
add_subdirectory(kernel/workers)
add_subdirectory(kernel/kpkg_devkit)
add_subdirectory(registrar)
add_custom_target(clean_modules COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean &&
        cd ${CMAKE_BINARY_DIR} && make clean_labstor_kernel_server &&
        cd ${CMAKE_BINARY_DIR} && make clean_secure_shmem &&
        #cd ${CMAKE_BINARY_DIR} && make clean_kernel_worker &&
        cd ${CMAKE_BINARY_DIR} && make clean_kpkg_devkit)