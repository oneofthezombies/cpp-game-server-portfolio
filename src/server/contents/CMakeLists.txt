file(GLOB_RECURSE SERVER_CONTENTS_SOURCES "*.cc")
add_library(server_contents STATIC ${SERVER_CONTENTS_SOURCES})

target_include_directories(server_contents PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server_contents core)
