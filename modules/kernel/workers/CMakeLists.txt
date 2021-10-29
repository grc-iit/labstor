cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(KERNEL_SERVER_PATH ${CMAKE_SOURCE_DIR}/src/kernel_server)
add_custom_target(build_worker ALL COMMAND
        cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel && make
        CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(build_worker build_labstor_kernel_server labstor_kernel_client)
add_custom_target(clean_worker COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel && make clean)

add_library(worker_client_netlink netlink_client/worker_user_netlink.cpp)
add_dependencies(worker_client_netlink labstor_kernel_client)
target_link_libraries(worker_client_netlink labstor_kernel_client)