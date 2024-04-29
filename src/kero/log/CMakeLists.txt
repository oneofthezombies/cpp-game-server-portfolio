file(GLOB_RECURSE SOURCES "*.cc")
add_library(kero_log STATIC ${SOURCES})

target_include_directories(kero_log PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(kero_log PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
