cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dummy)

add_library(dummy_client dummy/client/dummy_client.cpp)
add_dependencies(dummy_client labstor_client_library)
target_link_libraries(dummy_client labstor_client_library)

add_library(dummy_server dummy/server/dummy_server.cpp)
add_dependencies(dummy_server labstor_server_library)
target_link_libraries(dummy_server labstor_server_library)

add_executable(integration_exec test.cpp)
add_dependencies(integration_exec dummy_client)
target_link_libraries(integration_exec dummy_client)

add_custom_target(start_integration_server COMMAND ${CMAKE_BINARY_DIR}/labstor_trusted_server ${CMAKE_CURRENT_SOURCE_DIR}/config/config.yaml)