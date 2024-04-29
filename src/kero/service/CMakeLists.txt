file(GLOB_RECURSE SOURCES "*.cc")
add_library(kero_service STATIC ${SOURCES})

target_include_directories(kero_service PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(kero_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
