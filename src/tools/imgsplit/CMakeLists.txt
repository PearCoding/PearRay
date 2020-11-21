add_executable(prsplit main.cpp)
target_link_libraries(prsplit PRIVATE std::filesystem OpenImageIO::OpenImageIO)
add_lto(prsplit)
strip_binary(prsplit)

install(TARGETS prsplit COMPONENT runtime)
