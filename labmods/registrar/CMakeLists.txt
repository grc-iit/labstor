cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

set(MODULE_NAME registrar)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/modules ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

#BUILD KERNEL MODULE
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/kernel)
    set(KERNEL_SERVER_PATH ${CMAKE_SOURCE_DIR}/src/kernel/server)
    add_custom_target(build_${MODULE_NAME} ALL COMMAND
            cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel && make
            CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(build_${MODULE_NAME} build_labstor_kernel_server)
    add_custom_target(clean_${MODULE_NAME} COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/kernel && make clean)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kernel/${MODULE_NAME}.ko DESTINATION ${CMAKE_INSTALL_PREFIX}/kernel)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/kernel/${MODULE_NAME}_kernel.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/kernel)
endif()

#BUILD NETLINK CLIENT
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/netlink_client)
    add_library(${MODULE_NAME}_client_netlink
            netlink_client/${MODULE_NAME}_client_netlink.cpp)
    add_dependencies(${MODULE_NAME}_client_netlink
            labstor_kernel_client)
    target_link_libraries(${MODULE_NAME}_client_netlink
            labstor_kernel_client)
    install(TARGETS ${MODULE_NAME}_client_netlink DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/netlink_client/${MODULE_NAME}_client_netlink.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/netlink_client)
endif()

#BUILD USERSPACE CLIENT
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/client)
    add_library(${MODULE_NAME}_client client/${MODULE_NAME}_client.cpp)
    install(TARGETS ${MODULE_NAME}_client DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/client/${MODULE_NAME}_client.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/client)
endif()

#BUILD USERSPACE SERVER
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/server)
    add_library(${MODULE_NAME}_server server/${MODULE_NAME}_server.cpp)
    install(TARGETS ${MODULE_NAME}_server DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/server/${MODULE_NAME}_server.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/server)
endif()

