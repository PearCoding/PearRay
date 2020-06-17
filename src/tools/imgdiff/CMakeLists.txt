add_executable(prdiff main.cpp)
target_link_libraries(prdiff PRIVATE pr_lib_base Boost::filesystem Boost::program_options OpenImageIO::OpenImageIO)
add_lto(prdiff)
strip_binary(prdiff)
