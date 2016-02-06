CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_UTILS ${PR_Root}/utils)

SET(PR_Main_Src
  ${PR_UTILS}/Environment.cpp
  ${PR_UTILS}/SceneLoader.cpp)
SET(PR_GSL_Src
  ${PR_UTILS}/gsl/GSLMaterial.cpp)
SOURCE_GROUP("Source Files\\GSL" FILES ${PR_GSL_Src})
SET(PR_Loader_Src
  ${PR_UTILS}/loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Loader_Src} ${PR_GSL_Src}
)

SET(PR_Main_Hdr
  ${PR_UTILS}/Environment.h
  ${PR_UTILS}/SceneLoader.h)
SET(PR_GSL_Hdr
  ${PR_UTILS}/gsl/GSLNode.h
  ${PR_UTILS}/gsl/GSLMaterial.h)
SOURCE_GROUP("Header Files\\GSL" FILES ${PR_GSL_Hdr})
SET(PR_Loader_Hdr
  ${PR_UTILS}/loader/MeshLoader.h
  ${PR_UTILS}/loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Loader_Hdr} ${PR_GSL_Hdr}
)
	
add_library(pr_lib_utils SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_utils ${PR_TARGET})
target_link_libraries(pr_lib_utils ${DL_TARGET})
set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_lib_utils PROPERTIES COMPILE_DEFINITIONS "${DEFS};PR_LIB_UTILS_BUILD;" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")