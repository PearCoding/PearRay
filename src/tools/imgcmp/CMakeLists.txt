add_executable(prcmp main.cpp)
target_link_libraries(prcmp PRIVATE Boost::filesystem OpenImageIO::OpenImageIO)
add_lto(prcmp)
strip_binary(prcmp)
