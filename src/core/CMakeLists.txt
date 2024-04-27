file(GLOB_RECURSE CORE_SOURCES "*.cc")
add_library(core STATIC ${CORE_SOURCES})

target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
