file(GLOB_RECURSE SERVER_ENGINE_SOURCES "*.cc")
add_library(server_engine STATIC ${SERVER_ENGINE_SOURCES})

target_include_directories(server_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(server_engine PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(server_engine core)
