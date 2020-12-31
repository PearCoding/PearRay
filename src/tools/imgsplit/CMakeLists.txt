add_executable(prsplit main.cpp)
target_link_libraries(prsplit PRIVATE std::filesystem OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(prsplit PRIVATE OpenEXR::IlmImf)
endif()
add_lto(prsplit)
strip_binary(prsplit)

install(TARGETS prsplit COMPONENT runtime)
