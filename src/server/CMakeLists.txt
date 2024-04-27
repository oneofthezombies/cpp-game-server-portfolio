add_subdirectory(engine)
add_subdirectory(contents)

add_executable(server main.cc)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                          ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(server server_engine server_contents)
