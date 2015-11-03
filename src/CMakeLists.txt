CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  ${PR_Root}/FileLogListener.cpp
  ${PR_Root}/Logger.cpp
  ${PR_Root}/main.cpp)
SET(PR_Entity_Src
  ${PR_Root}/entity/Entity.cpp
  ${PR_Root}/entity/MeshEntity.cpp
  ${PR_Root}/entity/SphereEntity.cpp)
SOURCE_GROUP("Source Files\\Entity" FILES ${PR_Entity_Src})
SET(PR_Geometry_Src
  ${PR_Root}/geometry/BoundingBox.cpp
  ${PR_Root}/geometry/Face.cpp
  ${PR_Root}/geometry/Mesh.cpp
  ${PR_Root}/geometry/Normal.cpp
  ${PR_Root}/geometry/UV.cpp
  ${PR_Root}/geometry/Vertex.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_Loader_Src
  ${PR_Root}/loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Material_Src
  ${PR_Root}/material/DiffuseMaterial.cpp)
SOURCE_GROUP("Source Files\\Material" FILES ${PR_Material_Src})
SET(PR_Ray_Src
  ${PR_Root}/ray/Ray.cpp)
SOURCE_GROUP("Source Files\\Ray" FILES ${PR_Ray_Src})
SET(PR_Renderer_Src
  ${PR_Root}/renderer/RenderResult.cpp
  ${PR_Root}/renderer/Renderer.cpp)
SOURCE_GROUP("Source Files\\Renderer" FILES ${PR_Renderer_Src})
SET(PR_Scene_Src
  ${PR_Root}/scene/Camera.cpp
  ${PR_Root}/scene/Scene.cpp)
SOURCE_GROUP("Source Files\\Scene" FILES ${PR_Scene_Src})
SET(PR_Spectral_Src
  ${PR_Root}/spectral/IntensityConverter.cpp
  ${PR_Root}/spectral/RGBConverter.cpp
  ${PR_Root}/spectral/Spectrum.cpp)
SOURCE_GROUP("Source Files\\Spectral" FILES ${PR_Spectral_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
  ${PR_Entity_Src} ${PR_Ray_Src} ${PR_Scene_Src}
  ${PR_Renderer_Src} ${PR_Spectral_Src} ${PR_Material_Src}
)

SET(PR_Main_Hdr
  ${PR_Root}/Config.h.in
  ${PR_Root}/FileLogListener.h
  ${PR_Root}/Logger.h
  ${PR_Root}/LogListener.h)
SET(PR_Entity_Hdr
  ${PR_Root}/entity/Entity.h
  ${PR_Root}/entity/MeshEntity.h
  ${PR_Root}/entity/SphereEntity.h)
SOURCE_GROUP("Header Files\\Entity" FILES ${PR_Entity_Hdr})
SET(PR_Geometry_Hdr
  ${PR_Root}/geometry/BoundingBox.h
  ${PR_Root}/geometry/Face.h
  ${PR_Root}/geometry/Mesh.h
  ${PR_Root}/geometry/Normal.h
  ${PR_Root}/geometry/UV.h
  ${PR_Root}/geometry/Vertex.h)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_Loader_Hdr
  ${PR_Root}/loader/MeshLoader.h
  ${PR_Root}/loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Material_Hdr
  ${PR_Root}/material/DiffuseMaterial.h
  ${PR_Root}/material/Material.h)
SOURCE_GROUP("Header Files\\Material" FILES ${PR_Material_Hdr})
SET(PR_Ray_Hdr
  ${PR_Root}/ray/Ray.h)
SOURCE_GROUP("Header Files\\Ray" FILES ${PR_Ray_Hdr})
SET(PR_Renderer_Hdr
  ${PR_Root}/renderer/RenderResult.h
  ${PR_Root}/renderer/Renderer.h)
SOURCE_GROUP("Header Files\\Renderer" FILES ${PR_Renderer_Hdr})
SET(PR_Scene_Hdr
  ${PR_Root}/scene/Camera.h
  ${PR_Root}/scene/Scene.h)
SOURCE_GROUP("Header Files\\Scene" FILES ${PR_Scene_Hdr})
SET(PR_Spectral_Hdr
  ${PR_Root}/spectral/ColorSpaceConverter.h
  ${PR_Root}/spectral/IntensityConverter.h
  ${PR_Root}/spectral/RGBConverter.h
  ${PR_Root}/spectral/Spectrum.h)
SOURCE_GROUP("Header Files\\Spectral" FILES ${PR_Spectral_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Geometry_Hdr} ${PR_Loader_Hdr}
  ${PR_Entity_Hdr} ${PR_Ray_Hdr} ${PR_Scene_Hdr}
  ${PR_Renderer_Hdr} ${PR_Spectral_Hdr} ${PR_Material_Hdr}
)

SET(PR_TARGET pearray)

add_executable(${PR_TARGET} ${PR_Src} ${PR_Hdr})
target_link_libraries(${PR_TARGET} ${DL_TARGET})
target_link_libraries(${PR_TARGET} ${PP_TARGET})
set_target_properties(${PR_TARGET} PROPERTIES VERSION ${PR_Version})
set_target_properties(${PR_TARGET} PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")