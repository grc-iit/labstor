cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)
option(WITH_SPDK "Include SPDK tests" OFF)

#SHMEM latency
add_executable(test_usr_usr_ipc_thrpt src/ipc_thrpt/test_ipc_thrpt.cpp)
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

add_executable(labfs_md_emu src/md/metadata.cpp)

#MD Throughput
add_executable(test_md_thrpt src/posix_md/posix.cpp)