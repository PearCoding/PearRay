add_executable(prsplitprc main.cpp)
target_link_libraries(prsplitprc PRIVATE Boost::program_options Boost::filesystem)
add_lto(prsplitprc)
strip_binary(prsplitprc)
