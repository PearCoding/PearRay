CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

add_definitions(-DPR_PLUGIN_VERSION="1.0")
add_subdirectory(cameras)
add_subdirectory(entities)
add_subdirectory(materials)
add_subdirectory(integrators)
add_subdirectory(emissions)