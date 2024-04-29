file(GLOB_RECURSE SOURCES "*.cc")
add_library(kero_core STATIC ${SOURCES})

target_include_directories(kero_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(kero_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
