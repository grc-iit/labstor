cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include kernel io_systems)

######MODULES
add_subdirectory(kernel/secure_shmem)
add_subdirectory(kernel/ipc_manager)
add_subdirectory(kernel/work_orchestrator)
add_subdirectory(kernel/blkdev_table)
add_subdirectory(kernel/mq_driver)
add_subdirectory(kernel/ipc_test)
add_subdirectory(registrar)
add_subdirectory(dummy)
add_custom_target(clean_modules COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean &&
        cd ${CMAKE_BINARY_DIR} && make clean_labstor_kernel_server &&
        cd ${CMAKE_BINARY_DIR} && make clean_secure_shmem &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_manager &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_test &&
        cd ${CMAKE_BINARY_DIR} && make clean_blkdev_table &&
        cd ${CMAKE_BINARY_DIR} && make clean_mq_driver &&
        cd ${CMAKE_BINARY_DIR} && make clean_work_orchestrator)