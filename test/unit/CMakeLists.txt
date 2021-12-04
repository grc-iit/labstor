cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include ${CMAKE_SOURCE_DIR}/modules/kernel)

######BASICS
add_executable(test_strings strings/test.cpp)
add_executable(test_path_parser path_parser/test.cpp)

######CLIENT -> TRUSTED SERVER (SOCKET)
add_executable(client_to_trusted_server_exec sockets/client_to_trusted_server/test_client.cpp)
add_dependencies(client_to_trusted_server_exec labstor_client_library)
target_link_libraries(client_to_trusted_server_exec labstor_client_library)

######CLIENT -> KERNEL SERVER (SOCKET)
add_executable(client_to_kernel_exec sockets/client_to_kernel/test_kernel_client.cpp)
add_dependencies(client_to_kernel_exec labstor_kernel_client)
target_link_libraries(client_to_kernel_exec labstor_kernel_client)

######IN-MEMORY REQUEST QUEUE
add_executable(test_request_queue_exec request_queue/single/request_queue.cpp)

######SHARED MEMORY CREATION
add_executable(test_shmem_exec shared_memory/test_shmem.cpp)
add_dependencies(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_exec labstor_kernel_client secure_shmem_client_netlink)
add_custom_target(test_shmem ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_exec)

######UNORDERED MAP
add_executable(test_shmem_unordered_map_exec unordered_map/client/test.cpp)
add_executable(test_shmem_unordered_map2_exec unordered_map/client_client/test.cpp)
add_dependencies(test_shmem_unordered_map2_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem_unordered_map2_exec labstor_kernel_client secure_shmem_client_netlink mpi)
add_custom_target(test_multicore_map mpirun -n 2 ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_unordered_map2_exec)

######MEMORY ALLOCATION
add_executable(test_single_core_mem_alloc_exec memory_allocator/single/test.cpp)
add_executable(test_multicore_mem_alloc_exec memory_allocator/multicore/test.cpp)
add_dependencies(test_multicore_mem_alloc_exec labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_multicore_mem_alloc_exec labstor_kernel_client secure_shmem_client_netlink mpi)
add_custom_target(test_multicore_mem_alloc mpirun -n 4 ${CMAKE_CURRENT_BINARY_DIR}/test_multicore_mem_alloc_exec MULTICORE)

######SHARED MEMORY REQUEST QUEUE (USER - USER)
add_executable(test_shmem_request_queue_exec request_queue/client_client/shmem_request_queue.cpp)
add_dependencies(test_shmem_request_queue_exec labstor_server_library)
target_link_libraries(test_shmem_request_queue_exec labstor_server_library mpi)
add_custom_target(test_shmem_request_queue_path echo ${CMAKE_CURRENT_BINARY_DIR}/test_shmem_request_queue_exec)

######SHARED MEMORY REQUEST QUEUE (USER - KERNEL)
#add_subdirectory(request_queue/client_kernel)

######MODULE MANAGER
add_executable(test_module_manager_exec module_manager/test.cpp)
add_dependencies(test_module_manager_exec labstor_server_library)
target_link_libraries(test_module_manager_exec rt dl labstor_server_library)
target_include_directories(test_module_manager_exec PUBLIC module_manager)
add_library(simple_module SHARED module_manager/simple_module.cpp)
add_custom_target(test_module_manager ${CMAKE_CURRENT_BINARY_DIR}/test_module_manager_exec ${CMAKE_CURRENT_BINARY_DIR}/libsimple_module.so)

######CLEAN ALL TESTS
add_custom_target(clean_tests COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean_build_request_queue_client_kernel)