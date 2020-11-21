add_executable(prdiff main.cpp)
target_link_libraries(prdiff PRIVATE pr_lib_base std::filesystem OpenImageIO::OpenImageIO)
add_lto(prdiff)
strip_binary(prdiff)

install(TARGETS prdiff COMPONENT runtime)