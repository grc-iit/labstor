cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include ${CMAKE_SOURCE_DIR}/modules/kernel)

######CLIENT -> TRUSTED SERVER (SOCKET)
add_executable(client_to_trusted_server_exec unit/sockets/client_to_trusted_server/test_client.cpp)
add_dependencies(client_to_trusted_server_exec labstor_client_library)
target_link_libraries(client_to_trusted_server_exec labstor_client_library)

######CLIENT -> KERNEL SERVER (SOCKET)
add_executable(client_to_kernel_exec unit/sockets/client_to_kernel/test_kernel_client.cpp)
add_dependencies(client_to_kernel_exec labstor_kernel_client)
target_link_libraries(client_to_kernel_exec labstor_kernel_client)

######MEMORY ALLOCATION
add_executable(test_private_mem_alloc_exec unit/memory_allocator/single/test.cpp)

######SHARED MEMORY CREATION
add_executable(test_shmem_exec unit/shared_memory/test_shmem.cpp)
add_dependencies(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
add_custom_target(test_shmem ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_exec)

######IN-MEMORY REQUEST QUEUE
add_executable(request_queue_exec unit/request_queue/single/request_queue.cpp)
add_custom_target(test_request_queue ${CMAKE_CURRENT_BINARY_DIR}/request_queue)

######SHARED MEMORY REQUEST QUEUE (USER - USER)
add_executable(test_shmem_request_queue_exec unit/request_queue/client_client/shmem_request_queue.cpp)
add_dependencies(test_shmem_request_queue_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_request_queue_exec labstor_kernel_client secure_shmem_client_netlink mpi)
add_custom_target(test_shmem_request_queue_path echo ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_request_queue_exec)

######MODULE MANAGER
add_executable(test_module_manager_exec unit/module_manager/test.cpp ${CMAKE_SOURCE_DIR}/src/userspace_server/module_manager.cpp)
target_link_libraries(test_module_manager_exec rt dl)
target_include_directories(test_module_manager_exec PUBLIC unit/module_manager)
add_library(simple_module SHARED unit/module_manager/simple_module.cpp)
add_custom_target(test_module_manager ${CMAKE_CURRENT_BINARY_DIR}/test_module_manager_exec ${CMAKE_CURRENT_BINARY_DIR}/libsimple_module.so)

######SHARED MEMORY REQUEST QUEUE (USER - KERNEL)
add_subdirectory(unit/request_queue/client_kernel)

######CLEAN ALL TESTS
add_custom_target(clean_tests COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean_build_request_queue_client_kernel)