cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

#SHMEM latency
add_executable(test_usr_usr_ipc_thrpt src/ipc_thrpt/test_ipc_thrpt.cpp)
target_compile_options(test_usr_usr_ipc_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client)
target_link_libraries(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client "${OpenMP_CXX_FLAGS}")

#IO throughput
add_executable(test_io_thrpt src/io_thrpt/test.cpp)
target_compile_options(test_io_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client)
target_link_libraries(test_io_thrpt
        labstor_client_library
        mq_driver_client
        blkdev_table_client
        "${OpenMP_CXX_FLAGS}"
        rt uring aio
        pthread m dl numa

        spdk_nvme spdk_sock spdk_sock_posix spdk_trace spdk_rpc spdk_jsonrpc spdk_json spdk_util spdk_log
        spdk_env_dpdk spdk_util spdk_log rte_eal rte_mempool rte_ring rte_mbuf rte_bus_pci rte_pci
        rte_mempool_ring rte_telemetry rte_kvargs rte_power rte_ethdev rte_net rte_vhost rte_cryptodev
        rte_hash rte_rcu uuid)

#https://spdk.io/doc/pkgconfig.html

#IO Uring probe
add_executable(probe_uring src/io_thrpt/probe.cpp)
target_link_libraries(probe_uring rt uring)