cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)

include_directories(include ${CMAKE_SOURCE_DIR}/labmods/kernel)
set(SPDK_DEPS spdk_nvme_lib spdk_client)
set(SPDK_LIBS -lnuma -ldl -pthread -lrt -luuid -lcrypto -lm -laio spdk_nvme_lib spdk_client)

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

######BITMAP
add_executable(test_bitmap_exec bitmap/test_mpsc.cpp)
target_compile_options(test_bitmap_exec PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_bitmap_exec "${OpenMP_CXX_FLAGS}")

######BIT2MAP
add_executable(test_bit2map_exec bit2map/test_mpsc.cpp)
target_compile_options(test_bit2map_exec PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_bit2map_exec "${OpenMP_CXX_FLAGS}")

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

#add_executable(test_shmem_unordered_map_mpmc unordered_map/mpmc/test.cpp)
#target_compile_options(test_shmem_unordered_map_mpmc PUBLIC "${OpenMP_CXX_FLAGS}")
#target_link_libraries(test_shmem_unordered_map_mpmc "${OpenMP_CXX_FLAGS}")

######UNORDERED MAP FIND
add_executable(test_shmem_unordered_map_find_exec ipc_manager/server/test.cpp)

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

#add_executable(test_shmem_queue_mpmc request_queue/mpmc/test.cpp)
#target_compile_options(test_shmem_queue_mpmc PUBLIC "${OpenMP_CXX_FLAGS}")
#target_link_libraries(test_shmem_queue_mpmc "${OpenMP_CXX_FLAGS}")

#add_executable(test_shmem_qp_mpmc queue_pair/test.cpp)
#target_compile_options(test_shmem_qp_mpmc PUBLIC "${OpenMP_CXX_FLAGS}")
#target_link_libraries(test_shmem_qp_mpmc "${OpenMP_CXX_FLAGS}")

add_executable(test_shmem_qp_threaded queue_pair/test_threaded.cpp)
target_compile_options(test_shmem_qp_threaded PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_shmem_qp_threaded "${OpenMP_CXX_FLAGS}")

######MODULE MANAGER
add_executable(test_module_manager_exec module_manager/test.cpp)
add_dependencies(test_module_manager_exec labstor_server_library)
target_link_libraries(test_module_manager_exec rt dl labstor_server_library)
target_include_directories(test_module_manager_exec PUBLIC module_manager)
add_library(simple_module SHARED module_manager/simple_module.cpp)
add_custom_target(test_module_manager ${CMAKE_CURRENT_BINARY_DIR}/test_module_manager_exec ${CMAKE_CURRENT_BINARY_DIR}/libsimple_module.so)

#######THREAD LOCAL
add_executable(test_thread_local thread_local/test.cpp)
target_compile_options(test_thread_local PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_thread_local "${OpenMP_CXX_FLAGS}")

#######CHARVEC
add_executable(test_charvec charvec/test.cpp)

#######PROC PARTITION
add_executable(test_proc_partition proc_partition/test.cpp)

#######FILESYSTEM
add_executable(test_fs_preload filesystem/test.cpp)

#######SPDK
if(${WITH_SPDK})
    add_executable(test_spdk_lib spdk/test.cpp)
    add_dependencies(test_spdk_lib ${SPDK_DEPS})
    target_link_libraries(test_spdk_lib ${SPDK_LIBS})

    add_executable(test_spdk_module spdk/module.cpp)
    add_dependencies(test_spdk_module labstor_client_library ${SPDK_DEPS})
    target_link_libraries(test_spdk_module labstor_client_library ${SPDK_LIBS})
endif()

######CLEAN ALL TESTS
add_custom_target(clean_tests COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean_build_request_queue_client_kernel)