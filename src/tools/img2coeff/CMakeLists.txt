add_executable(primg2coeff main.cpp)
target_link_libraries(primg2coeff PRIVATE pr_lib_loader OpenImageIO::OpenImageIO)
add_lto(primg2coeff)
strip_binary(primg2coeff)
