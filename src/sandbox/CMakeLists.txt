
set(PR_Main_Src
  image_io.cpp
  main.cpp
  ProgramSettings.cpp
  suite_material.cpp
  suite_principled.cpp
  suite_projection.cpp
  suite_random.cpp
  suite_spectral.cpp)
set(PR_Src ${PR_Main_Src}
)

set(PR_Main_Hdr
  image_io.h
  ProgramSettings.h)
set(PR_Hdr ${PR_Main_Hdr}
)

add_executable(prsandbox ${PR_Src} ${PR_Hdr})
target_link_libraries(prsandbox PRIVATE pr_lib_loader OpenImageIO::OpenImageIO)
target_include_directories(prsandbox PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
add_lto(prsandbox)
strip_binary(prsandbox)

install(TARGETS prsandbox COMPONENT development EXCLUDE_FROM_ALL) # Make it optional