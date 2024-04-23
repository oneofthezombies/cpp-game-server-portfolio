file(GLOB_RECURSE SOURCES "*.cc")
add_executable(client ${SOURCES})

target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(client core)
