cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)

#SHMEM latency
add_executable(test_usr_usr_ipc_thrpt src/ipc_thrpt/test_ipc_thrpt.cpp)
target_compile_options(test_usr_usr_ipc_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client)
target_link_libraries(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client "${OpenMP_CXX_FLAGS}")

#IO throughput
add_executable(test_io_thrpt src/io_thrpt/test.cpp)
target_compile_options(test_io_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client)
target_link_libraries(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client "${OpenMP_CXX_FLAGS}" rt uring)

#IO Uring probe
add_executable(probe_uring src/io_thrpt/probe.cpp)
target_link_libraries(probe_uring rt uring)