cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

set(MODULE_NAME generic_block)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

#BUILD NETLINK CLIENT
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/netlink_client)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/netlink_client/${MODULE_NAME}_client_netlink.h
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include/labmods/${MODULE_NAME}/netlink_client)
endif()

#BUILD USERSPACE CLIENT
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/client)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/client/${MODULE_NAME}_client.h
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include/labmods/${MODULE_NAME}/client)
endif()

#BUILD USERSPACE SERVER
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/server)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/server/${MODULE_NAME}_server.h
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include/labmods/${MODULE_NAME}/server)
endif()