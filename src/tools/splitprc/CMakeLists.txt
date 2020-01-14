set(SRC
  main.cpp
  ProgramSettings.cpp
  ProgramSettings.h)

add_executable(prsplitprc ${SRC})
target_link_libraries(prsplitprc PRIVATE Boost::program_options Boost::filesystem)
add_lto(prsplitprc)
strip_binary(prsplitprc)
