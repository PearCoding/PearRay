set(PR_Main_Src
  main.cpp
  ProgramSettings.cpp)
set(PR_Src ${PR_Main_Src}
)

set(PR_Main_Hdr
  EnumOption.h
  ProgramSettings.h)
set(PR_Hdr ${PR_Main_Hdr}
)

include_directories(../utils)

add_executable(pearray ${PR_Src} ${PR_Hdr})
target_link_libraries(pearray PRIVATE pr_lib_loader Boost::program_options)
add_lto(pearray)
strip_binary(pearray)
