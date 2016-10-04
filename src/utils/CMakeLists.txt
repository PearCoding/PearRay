CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  Environment.cpp
  SceneLoader.cpp)
SET(PR_Loader_Src
  loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Parser_Entity_Src
  parser/entity/BoundaryParser.cpp
  parser/entity/CameraParser.cpp
  parser/entity/MeshParser.cpp
  parser/entity/PlaneParser.cpp
  parser/entity/SphereParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Entity" FILES ${PR_Parser_Entity_Src})
SET(PR_Parser_Light_Src
  parser/light/DistantLightParser.cpp
  parser/light/EnvironmentLightParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Light" FILES ${PR_Parser_Light_Src})
SET(PR_Parser_Material_Src
  parser/material/BlinnPhongMaterialParser.cpp
  parser/material/DiffuseMaterialParser.cpp
  parser/material/GlassMaterialParser.cpp
  parser/material/GridMaterialParser.cpp
  parser/material/MirrorMaterialParser.cpp
  parser/material/OrenNayarMaterialParser.cpp
  parser/material/WardMaterialParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Material" FILES ${PR_Parser_Material_Src})
SET(PR_Parser_Mesh_Src
  parser/mesh/TriMeshInlineParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Mesh" FILES ${PR_Parser_Mesh_Src})
SET(PR_Parser_Texture_Src
  parser/texture/TextureParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Texture" FILES ${PR_Parser_Texture_Src})
SET(PR_Renderer_Src
  renderer/DisplayBuffer.cpp)
SOURCE_GROUP("Source Files\\Renderer" FILES ${PR_Renderer_Src})
SET(PR_Shader_Src
  shader/ConstScalarOutput.cpp
  shader/ConstSpectralOutput.cpp
  shader/ConstVectorOutput.cpp
  shader/ImageScalarOutput.cpp
  shader/ImageSpectralOutput.cpp
  shader/ImageVectorOutput.cpp)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Loader_Src} ${PR_Parser_Entity_Src} ${PR_Parser_Material_Src}
  ${PR_Shader_Src} ${PR_Renderer_Src} ${PR_Parser_Mesh_Src} ${PR_Parser_Light_Src}
  ${PR_Parser_Texture_Src}
)

SET(PR_Main_Hdr
  Environment.h
  SceneLoader.h)
SET(PR_Loader_Hdr
  loader/SubGraphLoader.h
  loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Parser_Entity_Hdr
  parser/entity/BoundaryParser.h
  parser/entity/CameraParser.h
  parser/entity/EntityParser.h
  parser/entity/MeshParser.h
  parser/entity/PlaneParser.h
  parser/entity/SphereParser.h)
SOURCE_GROUP("Header Files\\Parser\\Entity" FILES ${PR_Parser_Entity_Hdr})
SET(PR_Parser_Light_Hdr
  parser/light/DistantLightParser.h
  parser/light/EnvironmentLightParser.h)
SOURCE_GROUP("Header Files\\Parser\\Light" FILES ${PR_Parser_Light_Hdr})
SET(PR_Parser_Material_Hdr
  parser/material/BlinnPhongMaterialParser.h
  parser/material/DiffuseMaterialParser.h
  parser/material/GlassMaterialParser.h
  parser/material/GridMaterialParser.h
  parser/material/MaterialParser.h
  parser/material/MirrorMaterialParser.h
  parser/material/OrenNayarMaterialParser.h
  parser/material/WardMaterialParser.h)
SOURCE_GROUP("Header Files\\Parser\\Material" FILES ${PR_Parser_Material_Hdr})
SET(PR_Parser_Mesh_Hdr
  parser/mesh/TriMeshInlineParser.h)
SOURCE_GROUP("Header Files\\Parser\\Mesh" FILES ${PR_Parser_Mesh_Hdr})
SET(PR_Parser_Texture_Hdr
  parser/texture/TextureParser.h)
SOURCE_GROUP("Header Files\\Parser\\Texture" FILES ${PR_Parser_Texture_Hdr})
SET(PR_Renderer_Hdr
  renderer/DisplayBuffer.h)
SOURCE_GROUP("Header Files\\Renderer" FILES ${PR_Renderer_Hdr})
SET(PR_Shader_Hdr
  shader/ConstScalarOutput.h
  shader/ConstSpectralOutput.h
  shader/ConstVectorOutput.h
  shader/ImageScalarOutput.h
  shader/ImageSpectralOutput.h
  shader/ImageVectorOutput.h)
SOURCE_GROUP("Header Files\\Shader" FILES ${PR_Shader_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Loader_Hdr} ${PR_Parser_Entity_Hdr} ${PR_Parser_Material_Hdr}
  ${PR_Shader_Hdr} ${PR_Renderer_Hdr} ${PR_Parser_Mesh_Hdr} ${PR_Parser_Light_Hdr}
  ${PR_Parser_Texture_Hdr}
)

if(OSL_FOUND)
  include_directories(../osl)
	set(DEFS "${DEFS};WITH_OSL")
endif()
	
add_library(pr_lib_utils SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_utils pr_lib)
target_link_libraries(pr_lib_utils ${DL_TARGET})
target_link_libraries(pr_lib_utils ${OIIO_LIBRARIES})

if(OSL_FOUND)
	target_link_libraries(pr_lib_utils pr_lib_osl)
endif()

set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib_utils PRIVATE "${DEFS};PR_LIB_UTILS_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")