cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

#Test performance of sched_getcpu()
add_executable(test_getcpu getcpu/test.cpp)

#Server throughput
add_executable(test_server_thrpt_exec server_thrpt/test.cpp)
add_dependencies(test_server_thrpt_exec labstor_client_library ipc_test_client)
target_link_libraries(test_server_thrpt_exec labstor_client_library ipc_test_client)