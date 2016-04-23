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
SET(PR_Parser_Src
  ${PR_UTILS}/parser/BoundaryParser.cpp
  ${PR_UTILS}/parser/CameraParser.cpp
  ${PR_UTILS}/parser/GridParser.cpp
  ${PR_UTILS}/parser/LightParser.cpp
  ${PR_UTILS}/parser/MeshParser.cpp
  ${PR_UTILS}/parser/PlaneParser.cpp
  ${PR_UTILS}/parser/SphereParser.cpp)
SOURCE_GROUP("Source Files\\Parser" FILES ${PR_Parser_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Loader_Src} ${PR_GSL_Src} ${PR_Parser_Src}
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
SET(PR_Parser_Hdr
  ${PR_UTILS}/parser/BoundaryParser.h
  ${PR_UTILS}/parser/CameraParser.h
  ${PR_UTILS}/parser/EntityParser.h
  ${PR_UTILS}/parser/GridParser.h
  ${PR_UTILS}/parser/LightParser.h
  ${PR_UTILS}/parser/MeshParser.h
  ${PR_UTILS}/parser/PlaneParser.h
  ${PR_UTILS}/parser/SphereParser.h)
SOURCE_GROUP("Header Files\\Parser" FILES ${PR_Parser_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Loader_Hdr} ${PR_GSL_Hdr} ${PR_Parser_Hdr}
)
	
add_library(pr_lib_utils SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_utils ${PR_TARGET})
target_link_libraries(pr_lib_utils ${DL_TARGET})
set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_lib_utils PROPERTIES COMPILE_DEFINITIONS "${DEFS};PR_LIB_UTILS_BUILD;" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")