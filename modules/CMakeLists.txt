cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include kernel io_systems)

######MODULES
add_subdirectory(kernel/secure_shmem)
add_subdirectory(kernel/workers)
add_custom_target(clean_all COMMAND
        make clean &&
        make clean_labstor_kernel_server &&
        make clean_secure_shmem &&
        make clean_kernel_worker)
add_dependencies(clean_all clean_labstor_kernel_server)