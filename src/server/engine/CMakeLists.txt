file(GLOB_RECURSE SERVER_SOURCES "*.cc")
add_executable(server ${SERVER_SOURCES})

target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(server core)
