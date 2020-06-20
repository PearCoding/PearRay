add_executable(prtonemap main.cpp)
target_link_libraries(prtonemap PRIVATE pr_lib_base Boost::filesystem OpenImageIO::OpenImageIO)
add_lto(prtonemap)
strip_binary(prtonemap)
