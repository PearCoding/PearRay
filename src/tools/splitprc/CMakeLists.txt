add_executable(prsplitprc main.cpp)
target_link_libraries(prsplitprc PRIVATE Boost::filesystem)
add_lto(prsplitprc)
strip_binary(prsplitprc)
