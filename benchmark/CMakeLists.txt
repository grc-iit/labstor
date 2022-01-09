cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

#SHMEM latency
add_executable(test_usr_usr_ipc_thrpt src/ipc_thrpt/test_ipc_thrpt.cpp)
target_compile_options(test_usr_usr_ipc_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client)
target_link_libraries(test_usr_usr_ipc_thrpt labstor_client_library ipc_test_client "${OpenMP_CXX_FLAGS}")

#SPDK dummy lib
add_library(spdk_nvme_lib STATIC src/dummy.c)
set(SPDK_BUILD_DIR /home/cc/scspkg/packages/spdk/src/spdk/)
set(SPDK_COMPILE_OPTS
        -MMD -MP -MF -g -Wall -Wextra -Wno-unused-parameter
        -Wno-missing-field-initializers -Wmissing-declarations -fno-strict-aliasing
        -march=native -Wformat
        -Wformat-security -D_GNU_SOURCE -fPIC -fstack-protector -fno-common
        -DNDEBUG -O2 -U_FORTIFY_SOURCE
        -D_FORTIFY_SOURCE=2 -DSPDK_GIT_COMMIT=4e4f11ff7 -pthread -Wno-pointer-sign
        -Wstrict-prototypes -std=gnu99)
set(SPDK_LINK_OPTS
        -Wl,-z,relro,-z,now -Wl,-z,noexecstack -fuse-ld=bfd
        -Wl,--whole-archive
        -Wl,--no-as-needed
        ${SPDK_BUILD_DIR}/build/lib/libspdk_sock_posix.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_nvme.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_sock.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_trace.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_rpc.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_jsonrpc.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_json.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_util.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_vmd.a
        ${SPDK_BUILD_DIR}/build/lib/libspdk_log.a
        -Wl,--no-whole-archive
        ${SPDK_BUILD_DIR}/build/lib/libspdk_env_dpdk.a
        -Wl,--whole-archive
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_eal.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_mempool.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_ring.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_mbuf.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_bus_pci.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_pci.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_mempool_ring.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_power.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_ethdev.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_net.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_telemetry.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_kvargs.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_vhost.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_cryptodev.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_hash.a
        ${SPDK_BUILD_DIR}/dpdk/build/lib/librte_rcu.a
        -Wl,--no-whole-archive  -lnuma -ldl
        -L${SPDK_BUILD_DIR}/isa-l/.libs -lisal -pthread -lrt -luuid
        -lcrypto -lm -laio)
target_compile_options(spdk_nvme_lib PUBLIC ${SPDK_COMPILE_OPTS})
target_link_options(spdk_nvme_lib PUBLIC ${SPDK_LINK_OPTS})
set(SPDK_DYN_LIBS -lnuma -ldl -pthread -lrt -luuid -lcrypto -lm -laio)

#SPDK test
add_executable(test_spdk src/spdk/hello.c)
add_dependencies(test_spdk spdk_nvme_lib)
target_link_libraries(test_spdk spdk_nvme_lib ${SPDK_DYN_LIBS})

#IO throughput
add_executable(test_io_thrpt src/io_thrpt/test.cpp)
target_compile_options(test_io_thrpt PUBLIC "${OpenMP_CXX_FLAGS}")
add_dependencies(test_io_thrpt labstor_client_library mq_driver_client blkdev_table_client spdk_nvme_lib)
target_link_libraries(test_io_thrpt
        labstor_client_library
        mq_driver_client
        blkdev_table_client
        "${OpenMP_CXX_FLAGS}"
        uring aio
        spdk_nvme_lib ${SPDK_DYN_LIBS})
#https://spdk.io/doc/pkgconfig.html

#IO Uring probe
add_executable(probe_uring src/io_thrpt/probe.cpp)
target_link_libraries(probe_uring rt uring)