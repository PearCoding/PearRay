add_executable(primg2coeff main.cpp)
target_link_libraries(primg2coeff PRIVATE pr_lib_loader OpenImageIO::OpenImageIO)
if(WIN32)
  target_link_libraries(primg2coeff PRIVATE OpenEXR::IlmImf)
endif()
add_lto(primg2coeff)
strip_binary(primg2coeff)

install(TARGETS primg2coeff COMPONENT runtime)