cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(KERNEL_SERVER_PATH ${CMAKE_SOURCE_DIR}/src/kernel/server)
add_custom_target(build_request_queue_client_kernel ALL COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && make
        CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(build_request_queue_client_kernel build_labstor_kernel_server)
add_custom_target(clean_build_request_queue_client_kernel COMMAND cd ../../request_queue_client_kernel && make clean)
add_executable(request_queue_client_kernel_exec request_queue_client.cpp)
add_dependencies(request_queue_client_kernel_exec secure_shmem_client_netlink labstor_kernel_client)
target_link_libraries(request_queue_client_kernel_exec secure_shmem_client_netlink labstor_kernel_client)

add_custom_target(request_queue_client_kernel_test ${CMAKE_CURRENT_BINARY_DIR}/request_queue_client_kernel_exec)