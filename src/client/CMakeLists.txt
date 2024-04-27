file(GLOB_RECURSE SOURCES "*.cc")
add_executable(client ${SOURCES})

target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(client core)
