add_executable(prdiff main.cpp)
target_link_libraries(prdiff PRIVATE pr_lib_base std::filesystem OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(prdiff PRIVATE OpenEXR::IlmImf)
endif()
add_lto(prdiff)
strip_binary(prdiff)

install(TARGETS prdiff COMPONENT runtime)