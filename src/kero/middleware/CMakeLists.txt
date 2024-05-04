file(GLOB_RECURSE SOURCES "*.cc")
add_library(kero_middleware STATIC ${SOURCES})

target_include_directories(kero_middleware PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(kero_middleware PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
