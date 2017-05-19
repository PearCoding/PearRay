
SET(PR_Main_Src
  main.cpp
  ProgramSettings.cpp
  suite_spectral1.cpp
  suite_random1.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr 
  ProgramSettings.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

if(OSL_FOUND)
  include_directories(../osl)
	set(DEFS "${DEFS};WITH_OSL")
endif()
	
add_executable(pr_sandbox ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_sandbox pr_lib)
target_link_libraries(pr_sandbox ${DL_TARGET})
target_link_libraries(pr_sandbox ${OIIO_LIBRARIES})

if(OSL_FOUND)
	target_link_libraries(pr_sandbox pr_lib_osl)
endif()

set_target_properties(pr_sandbox PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_sandbox PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")