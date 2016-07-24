CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_UTILS ${PR_Root}/utils)

SET(PR_Main_Src
  ${PR_UTILS}/Environment.cpp
  ${PR_UTILS}/SceneLoader.cpp)
SET(PR_Loader_Src
  ${PR_UTILS}/loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Parser_Entity_Src
  ${PR_UTILS}/parser/entity/BoundaryParser.cpp
  ${PR_UTILS}/parser/entity/CameraParser.cpp
  ${PR_UTILS}/parser/entity/MeshParser.cpp
  ${PR_UTILS}/parser/entity/PlaneParser.cpp
  ${PR_UTILS}/parser/entity/SphereParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Entity" FILES ${PR_Parser_Entity_Src})
SET(PR_Parser_Material_Src
  ${PR_UTILS}/parser/material/BlinnPhongMaterialParser.cpp
  ${PR_UTILS}/parser/material/DebugBoundingBoxMaterialParser.cpp
  ${PR_UTILS}/parser/material/DiffuseMaterialParser.cpp
  ${PR_UTILS}/parser/material/GlassMaterialParser.cpp
  ${PR_UTILS}/parser/material/GridMaterialParser.cpp
  ${PR_UTILS}/parser/material/MirrorMaterialParser.cpp
  ${PR_UTILS}/parser/material/OrenNayarMaterialParser.cpp
  ${PR_UTILS}/parser/material/WardMaterialParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Material" FILES ${PR_Parser_Material_Src})
SET(PR_Shader_Src
  ${PR_UTILS}/shader/ConstScalarOutput.cpp
  ${PR_UTILS}/shader/ConstSpectralOutput.cpp
  ${PR_UTILS}/shader/ConstVectorOutput.cpp
  ${PR_UTILS}/shader/ImageScalarOutput.cpp
  ${PR_UTILS}/shader/ImageSpectralOutput.cpp
  ${PR_UTILS}/shader/ImageVectorOutput.cpp)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Loader_Src} ${PR_Parser_Entity_Src} ${PR_Parser_Material_Src}
  ${PR_Shader_Src}
)

SET(PR_Main_Hdr
  ${PR_UTILS}/Environment.h
  ${PR_UTILS}/SceneLoader.h)
SET(PR_Loader_Hdr
  ${PR_UTILS}/loader/SubGraphLoader.h
  ${PR_UTILS}/loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Parser_Entity_Hdr
  ${PR_UTILS}/parser/entity/BoundaryParser.h
  ${PR_UTILS}/parser/entity/CameraParser.h
  ${PR_UTILS}/parser/entity/EntityParser.h
  ${PR_UTILS}/parser/entity/MeshParser.h
  ${PR_UTILS}/parser/entity/PlaneParser.h
  ${PR_UTILS}/parser/entity/SphereParser.h)
SOURCE_GROUP("Header Files\\Parser\\Entity" FILES ${PR_Parser_Entity_Hdr})
SET(PR_Parser_Material_Hdr
  ${PR_UTILS}/parser/material/BlinnPhongMaterialParser.h
  ${PR_UTILS}/parser/material/DebugBoundingBoxMaterialParser.h
  ${PR_UTILS}/parser/material/DiffuseMaterialParser.h
  ${PR_UTILS}/parser/material/GlassMaterialParser.h
  ${PR_UTILS}/parser/material/GridMaterialParser.h
  ${PR_UTILS}/parser/material/MaterialParser.h
  ${PR_UTILS}/parser/material/MirrorMaterialParser.h
  ${PR_UTILS}/parser/material/OrenNayarMaterialParser.h
  ${PR_UTILS}/parser/material/WardMaterialParser.h)
SOURCE_GROUP("Header Files\\Parser\\Material" FILES ${PR_Parser_Material_Hdr})
SET(PR_Shader_Hdr
  ${PR_UTILS}/shader/ConstScalarOutput.h
  ${PR_UTILS}/shader/ConstSpectralOutput.h
  ${PR_UTILS}/shader/ConstVectorOutput.h
  ${PR_UTILS}/shader/ImageScalarOutput.h
  ${PR_UTILS}/shader/ImageSpectralOutput.h
  ${PR_UTILS}/shader/ImageVectorOutput.h)
SOURCE_GROUP("Header Files\\Shader" FILES ${PR_Shader_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Loader_Hdr} ${PR_Parser_Entity_Hdr} ${PR_Parser_Material_Hdr}
  ${PR_Shader_Hdr}
)
	
add_library(pr_lib_utils SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_utils ${PR_TARGET})
target_link_libraries(pr_lib_utils ${DL_TARGET})
target_link_libraries(pr_lib_utils ${OIIO_LIBRARIES})
set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib_utils PRIVATE "${DEFS};PR_LIB_UTILS_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")