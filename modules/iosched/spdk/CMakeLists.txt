cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

set(MODULE_NAME spdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

#BUILD SHARED LIB FOR SPDK
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    #SPDK dummy lib
    add_library(spdk_nvme_lib STATIC lib/dummy.cpp)
    set(SPDK_DYN_LIBS "")
    set(SPDK_BUILD_DIR $ENV{SPDK_ROOT}/src/spdk/)
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

    add_library(${MODULE_NAME}_lib lib/${MODULE_NAME}_lib.cpp)
    add_dependencies(${MODULE_NAME}_lib spdk_nvme_lib)
    target_link_libraries(${MODULE_NAME}_lib ${SPDK_DYN_LIBS} spdk_nvme_lib)
endif()

#BUILD USERSPACE CLIENT
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/client2)
    add_library(${MODULE_NAME}_client client/${MODULE_NAME}_client.cpp)
    add_dependencies(${MODULE_NAME}_client labstor_client_library)
    target_link_libraries(${MODULE_NAME}_client labstor_client_library ${MODULE_NAME}_lib ${SPDK_DYN_LIBS} spdk_nvme_lib)
endif()

#BUILD USERSPACE SERVER
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/server2)
    add_library(${MODULE_NAME}_server server/${MODULE_NAME}_server.cpp)
    add_dependencies(${MODULE_NAME}_server labstor_server_library)
    target_link_libraries(${MODULE_NAME}_server labstor_server_library ${MODULE_NAME}_lib ${SPDK_DYN_LIBS} spdk_nvme_lib)
endif()

add_executable(spdk_probe util/probe.cpp)
add_dependencies(spdk_probe spdk_nvme_lib spdk_lib)
target_link_libraries(spdk_probe ${SPDK_DYN_LIBS} spdk_nvme_lib spdk_lib)