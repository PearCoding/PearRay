add_executable(prcontrol main.cpp)
target_link_libraries(prcontrol PRIVATE pr_lib_base OpenImageIO::OpenImageIO)
add_lto(prcontrol)
strip_binary(prcontrol)

install(TARGETS prcontrol)