CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  ${PR_Root}/console/main.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
)
SET(PR_Hdr ${PR_Main_Hdr}
)

add_executable(pr_c ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_c ${PR_TARGET})
target_link_libraries(pr_c ${DL_TARGET})
target_link_libraries(pr_c ${PP_TARGET})
set_target_properties(pr_c PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_c PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")