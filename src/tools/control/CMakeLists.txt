add_executable(prcontrol main.cpp)
target_link_libraries(prcontrol PRIVATE pr_lib_base OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(prcontrol PRIVATE OpenEXR::IlmImf)
endif()
add_lto(prcontrol)
strip_binary(prcontrol)

install(TARGETS prcontrol COMPONENT runtime)