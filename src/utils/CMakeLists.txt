CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  Environment.cpp
  SceneLoader.cpp
  SpectralFile.cpp)
SET(PR_Loader_Src
  loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Parser_Mesh_Src
  parser/mesh/TriMeshInlineParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Mesh" FILES ${PR_Parser_Mesh_Src})
SET(PR_Parser_Texture_Src
  parser/texture/TextureParser.cpp)
SOURCE_GROUP("Source Files\\Parser\\Texture" FILES ${PR_Parser_Texture_Src})
SET(PR_Output_Src
  output/ImageWriter.cpp
  output/OutputSpecification.cpp)
SOURCE_GROUP("Source Files\\Output" FILES ${PR_Output_Src})
SET(PR_Shader_Src
  shader/ImageShadingSocket.cpp)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Loader_Src} ${PR_Shader_Src}
  ${PR_Output_Src} ${PR_Parser_Mesh_Src} ${PR_Parser_Texture_Src}
)

SET(PR_Main_Hdr
  Environment.h
  SceneLoader.h
  SpectralFile.h)
SET(PR_Loader_Hdr
  loader/SubGraphLoader.h
  loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Parser_Mesh_Hdr
  parser/mesh/TriMeshInlineParser.h)
SOURCE_GROUP("Header Files\\Parser\\Mesh" FILES ${PR_Parser_Mesh_Hdr})
SET(PR_Parser_Texture_Hdr
  parser/texture/TextureParser.h)
SOURCE_GROUP("Header Files\\Parser\\Texture" FILES ${PR_Parser_Texture_Hdr})
SET(PR_Output_Hdr
  output/ImageWriter.h
  output/OutputSpecification.h)
SOURCE_GROUP("Header Files\\Output" FILES ${PR_Output_Hdr})
SET(PR_Shader_Hdr
  shader/ImageShadingSocket.h)
SOURCE_GROUP("Header Files\\Shader" FILES ${PR_Shader_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Loader_Hdr} ${PR_Shader_Hdr} 
  ${PR_Output_Hdr} ${PR_Parser_Mesh_Hdr} ${PR_Parser_Texture_Hdr}
)

add_library(pr_lib_utils SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_utils pr_lib)
target_link_libraries(pr_lib_utils ${DL_TARGET})
target_link_libraries(pr_lib_utils ${OIIO_LIBRARIES})

set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_lib_utils PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(pr_lib_utils PRIVATE "${DEFS};PR_LIB_UTILS_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")

IF(PR_CPP_CHECK)
  add_cpplint_target(pr_lib_utils_cppcheck "${PR_Src};${PR_Hdr}")
ENDIF()