
SET(PR_Main_Src
  image_io.cpp
  main.cpp
  ProgramSettings.cpp
  suite_material.cpp
  suite_principled.cpp
  suite_projection.cpp
  suite_random.cpp
  suite_spectral.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  image_io.h
  ProgramSettings.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

add_executable(pr_sandbox ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_sandbox PRIVATE pr_lib_utils)
target_link_libraries(pr_sandbox PRIVATE Boost::filesystem)
if(NOT WIN32)
  target_link_libraries(pr_sandbox PRIVATE Boost::program_options)
endif()
target_link_libraries(pr_sandbox PRIVATE OpenImageIO::Core OpenImageIO::Util)
target_include_directories(pr_sandbox PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
add_lto(pr_sandbox)
strip_binary(pr_sandbox)