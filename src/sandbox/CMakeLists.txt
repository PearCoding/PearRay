
set(PR_Main_Src
  image_io.cpp
  main.cpp
  ProgramSettings.cpp
  suite_material.cpp
  suite_principled.cpp
  suite_projection.cpp
  suite_random.cpp
  suite_spectral.cpp
  suite_triangle.cpp)
set(PR_Src ${PR_Main_Src}
)

set(PR_Main_Hdr
  image_io.h
  ProgramSettings.h)
set(PR_Hdr ${PR_Main_Hdr}
)

add_executable(pr_sandbox ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_sandbox PRIVATE pr_lib_loader Boost::program_options)
target_include_directories(pr_sandbox PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
add_lto(pr_sandbox)
strip_binary(pr_sandbox)