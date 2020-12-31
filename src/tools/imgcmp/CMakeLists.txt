add_executable(prcmp main.cpp)
target_link_libraries(prcmp PRIVATE std::filesystem OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(prcmp PRIVATE OpenEXR::IlmImf)
endif()
add_lto(prcmp)
strip_binary(prcmp)

install(TARGETS prcmp COMPONENT runtime)