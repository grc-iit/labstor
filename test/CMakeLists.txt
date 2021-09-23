cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include ${CMAKE_SOURCE_DIR}/modules/kernel)

######CLIENT -> TRUSTED SERVER
add_executable(client_to_trusted_server_exec unit/client_to_trusted_server/test_client.cpp)
add_dependencies(client_to_trusted_server_exec labstor_client_library)
target_link_libraries(client_to_trusted_server_exec labstor_client_library)

######CLIENT -> KERNEL SERVER
add_executable(client_to_kernel_exec unit/client_to_kernel/test_kernel_client.cpp)
add_dependencies(client_to_kernel_exec labstor_kernel_client)
target_link_libraries(client_to_kernel_exec labstor_kernel_client)

######SHARED MEMORY CREATION
add_executable(test_shmem_exec unit/test_shmem/test_shmem.cpp)
add_dependencies(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
add_custom_target(test_shmem ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_exec)

######IN-MEMORY REQUEST QUEUE
add_executable(request_queue_exec unit/request_queue/request_queue.cpp)
add_custom_target(test_request_queue ${CMAKE_CURRENT_BINARY_DIR}/request_queue)

######SHARED MEMORY REQUEST QUEUE
add_executable(test_shmem_request_queue_exec unit/shmem_request_queue/shmem_request_queue.cpp)
add_dependencies(test_shmem_request_queue_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_request_queue_exec labstor_kernel_client secure_shmem_client_netlink mpi)
add_custom_target(test_shmem_request_queue_path echo ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_request_queue_exec)