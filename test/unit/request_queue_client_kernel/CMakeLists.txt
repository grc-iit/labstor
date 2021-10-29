cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(KERNEL_SERVER_PATH ${CMAKE_SOURCE_DIR}/src/kernel_server)
add_custom_target(build_request_queue_client_kernel ALL COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel &&
        make KERNEL_SERVER=${KERNEL_SERVER_PATH} SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} LABSTOR_INCLUDE=${CMAKE_SOURCE_DIR}/include)
add_dependencies(build_request_queue_client_kernel build_labstor_kernel_server)
add_custom_target(clean_build_request_queue_client_kernel COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && make clean)
add_executable(request_queue_client_kernel_exec netlink_client/shmem_user_netlink.cpp)