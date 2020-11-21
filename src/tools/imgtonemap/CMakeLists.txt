add_executable(prtonemap main.cpp)
target_link_libraries(prtonemap PRIVATE pr_lib_base std::filesystem OpenImageIO::OpenImageIO)
add_lto(prtonemap)
strip_binary(prtonemap)

install(TARGETS prtonemap)
