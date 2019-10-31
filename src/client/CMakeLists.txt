SET(PR_Main_Src
  main.cpp
  ProgramSettings.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  EnumOption.h
  ProgramSettings.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

include_directories(../utils)

add_executable(pearray ${PR_Src} ${PR_Hdr})
target_link_libraries(pearray PRIVATE pr_lib)
target_link_libraries(pearray PRIVATE pr_lib_utils)
if(NOT WIN32)
	target_link_libraries(pearray PRIVATE Boost::program_options)
endif()
target_compile_definitions(pearray PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
add_lto(pearray)
strip_binary(pearray)
