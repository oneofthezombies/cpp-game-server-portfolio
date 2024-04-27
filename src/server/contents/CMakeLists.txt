file(GLOB_RECURSE SOURCES "*.cc")
add_library(server_contents STATIC ${SOURCES})

target_include_directories(server_contents PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(server_contents PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(server_contents PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(server_contents core)
