add_executable(prtonemap main.cpp)
target_link_libraries(prtonemap PRIVATE pr_lib_base std::filesystem OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(prtonemap PRIVATE OpenEXR::IlmImf)
endif()
add_lto(prtonemap)
strip_binary(prtonemap)

install(TARGETS prtonemap COMPONENT runtime)
