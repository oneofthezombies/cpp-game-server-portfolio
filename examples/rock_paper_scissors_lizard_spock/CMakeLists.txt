add_executable(client client.cc)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(client kero_core kero_log kero_engine)

add_executable(server server.cc)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(server kero_core kero_log kero_engine)
