file(GLOB_RECURSE SOURCES "*.cc")
add_library(kero_engine STATIC ${SOURCES})

target_include_directories(kero_engine PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(kero_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
