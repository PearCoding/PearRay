CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  RendererService.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  RendererService.h)
SET(PR_Hdr ${PR_Main_Hdr}
)
	
add_library(pr_lib_osl SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_osl pr_lib)
target_link_libraries(pr_lib_osl ${OIIO_LIBRARIES})
target_link_libraries(pr_lib_osl ${OSL_LIBRARIES})
set_target_properties(pr_lib_osl PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib_osl PRIVATE "${DEFS};PR_LIB_OSL_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")