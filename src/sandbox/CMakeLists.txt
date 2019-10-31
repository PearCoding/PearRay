
SET(PR_Main_Src
  main.cpp
  ProgramSettings.cpp
  suite_projection1.cpp
  suite_random1.cpp
  suite_spectral1.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  ProgramSettings.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

add_executable(pr_sandbox ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_sandbox PRIVATE pr_lib)
target_link_libraries(pr_sandbox PRIVATE Boost::filesystem)
if(NOT WIN32)
target_link_libraries(pr_sandbox PRIVATE Boost::program_options)
endif()
target_link_libraries(pr_sandbox PRIVATE ${DL_TARGET})
target_link_libraries(pr_sandbox PRIVATE ${OIIO_LIBRARIES})
target_link_libraries(pr_sandbox PRIVATE ${OPENEXR_LIBRARIES})
target_compile_definitions(pr_sandbox PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
add_lto(pr_sandbox)
strip_binary(pr_sandbox)