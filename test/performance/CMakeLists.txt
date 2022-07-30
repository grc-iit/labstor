cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)

#Test performance of sched_getcpu()
add_executable(test_getcpu getcpu/test.cpp)

#Atomic throughput
add_executable(test_atomic_thrpt atomics/test.cpp)

#Unordered map throughput
add_executable(test_map_thrpt unordered_map/test.cpp)

#Queue throughput
add_executable(test_queue_thrpt queue_thrpt/test.cpp)
target_link_libraries(test_queue_thrpt -pthread -lrt)

add_executable(test_queue_thrpt_threaded queue_thrpt/test_threaded.cpp)
target_compile_options(test_queue_thrpt_threaded PUBLIC "${OpenMP_CXX_FLAGS}")
target_link_libraries(test_queue_thrpt_threaded "${OpenMP_CXX_FLAGS}")

#Chrono
add_executable(test_chrono_exec chrono/test.cpp)

#SHMEM latency
add_executable(test_shmem_latency shmem/test.cpp)
target_compile_options(test_shmem_latency PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_shmem_latency labstor_client_library ipc_test_client)
target_link_libraries(test_shmem_latency labstor_client_library ipc_test_client "${OpenMP_CXX_FLAGS}")

#Md thrpt
add_executable(labfs_md_emu md_thrpt/labfs.cpp)
add_executable(test_md_thrpt md_thrpt/posix.cpp)

#SHMEM latency
add_executable(test_usr_usr_ipc_thrpt ipc_thrpt/test_ipc_thrpt.cpp)
target_compile_options(test_usr_usr_ipc_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client)
target_link_libraries(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client "${OpenMP_CXX_FLAGS}")

#IO throughput
#add_executable(test_io_thrpt src/io_thrpt/test.cpp)
#target_compile_options(test_io_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
#add_dependencies(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client spdk_nvme_lib)
#add_dependencies(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client)
#target_link_libraries(test_io_thrpt
#        labstor_client_library
#        mq_driver_client
#        blkdev_table_client
#        "${OpenMP_CXX_FLAGS}"
#        uring aio)
#spdk_nvme_lib ${SPDK_DYN_LIBS})
#https://spdk.io/doc/pkgconfig.html