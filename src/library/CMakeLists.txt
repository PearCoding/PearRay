CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_LIB ${PR_Root}/library)

SET(PR_Main_Src
  ${PR_LIB}/FileLogListener.cpp
  ${PR_LIB}/Logger.cpp)
SET(PR_Camera_Src
  ${PR_LIB}/camera/Camera.cpp
  ${PR_LIB}/camera/OrthographicCamera.cpp
  ${PR_LIB}/camera/PerspectiveCamera.cpp)
SOURCE_GROUP("Source Files\\Camera" FILES ${PR_Camera_Src})
SET(PR_Entity_Src
  ${PR_LIB}/entity/BoundaryEntity.cpp
  ${PR_LIB}/entity/Entity.cpp
  ${PR_LIB}/entity/MeshEntity.cpp
  ${PR_LIB}/entity/PlaneEntity.cpp
  ${PR_LIB}/entity/RenderEntity.cpp
  ${PR_LIB}/entity/SphereEntity.cpp)
SOURCE_GROUP("Source Files\\Entity" FILES ${PR_Entity_Src})
SET(PR_Geometry_Src
  ${PR_LIB}/geometry/BoundingBox.cpp
  ${PR_LIB}/geometry/Face.cpp
  ${PR_LIB}/geometry/FacePoint.cpp
  ${PR_LIB}/geometry/Mesh.cpp
  ${PR_LIB}/geometry/Plane.cpp
  ${PR_LIB}/geometry/Sphere.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_Lights_Src
  ${PR_LIB}/lights/PointLight.cpp)
SOURCE_GROUP("Source Files\\Lights" FILES ${PR_Lights_Src})
SET(PR_Material_Src
  ${PR_LIB}/material/BRDF.cpp
  ${PR_LIB}/material/BRDFMaterial.cpp
  ${PR_LIB}/material/DebugBoundingBoxMaterial.cpp
  ${PR_LIB}/material/DirectLightDebugMaterial.cpp
  ${PR_LIB}/material/GlassMaterial.cpp
  ${PR_LIB}/material/GridMaterial.cpp
  ${PR_LIB}/material/MirrorMaterial.cpp
  ${PR_LIB}/material/NormalDebugMaterial.cpp
  ${PR_LIB}/material/UVDebugMaterial.cpp)
SOURCE_GROUP("Source Files\\Material" FILES ${PR_Material_Src})
SET(PR_Photon_Src
  ${PR_LIB}/photon/PhotonMap.cpp)
SOURCE_GROUP("Source Files\\Photon" FILES ${PR_Photon_Src})
SET(PR_Ray_Src
  ${PR_LIB}/ray/Ray.cpp)
SOURCE_GROUP("Source Files\\Ray" FILES ${PR_Ray_Src})
SET(PR_Renderer_Src
  ${PR_LIB}/renderer/RenderResult.cpp
  ${PR_LIB}/renderer/Renderer.cpp
  ${PR_LIB}/renderer/RenderThread.cpp)
SOURCE_GROUP("Source Files\\Renderer" FILES ${PR_Renderer_Src})
SET(PR_Sampler_Src
  ${PR_LIB}/sampler/Stratified1DSampler.cpp
  ${PR_LIB}/sampler/Stratified2DSampler.cpp
  ${PR_LIB}/sampler/Stratified3DSampler.cpp)
SOURCE_GROUP("Source Files\\Sampler" FILES ${PR_Sampler_Src})
SET(PR_Scene_Src
  ${PR_LIB}/scene/kdTree.cpp
  ${PR_LIB}/scene/Scene.cpp)
SOURCE_GROUP("Source Files\\Scene" FILES ${PR_Scene_Src})
SET(PR_Spectral_Src
  ${PR_LIB}/spectral/IntensityConverter.cpp
  ${PR_LIB}/spectral/RGBConverter.cpp
  ${PR_LIB}/spectral/Spectrum.cpp
  ${PR_LIB}/spectral/XYZConverter.cpp)
SOURCE_GROUP("Source Files\\Spectral" FILES ${PR_Spectral_Src})
SET(PR_Thread_Src
  ${PR_LIB}/thread/Thread.cpp)
SOURCE_GROUP("Source Files\\Thread" FILES ${PR_Thread_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
  ${PR_Entity_Src} ${PR_Ray_Src} ${PR_Scene_Src}
  ${PR_Renderer_Src} ${PR_Spectral_Src} ${PR_Material_Src}
  ${PR_Thread_Src} ${PR_Camera_Src} ${PR_Lights_Src} ${PR_Sampler_Src}
  ${PR_Photon_Src}
)

SET(PR_Main_Hdr
  ${PR_LIB}/Config.h.in
  ${PR_LIB}/FileLogListener.h
  ${PR_LIB}/Logger.h
  ${PR_LIB}/LogListener.h
  ${PR_LIB}/Random.h)
SET(PR_Camera_Hdr
  ${PR_LIB}/camera/Camera.h
  ${PR_LIB}/camera/OrthographicCamera.h
  ${PR_LIB}/camera/PerspectiveCamera.h)
SOURCE_GROUP("Header Files\\Camera" FILES ${PR_Camera_Hdr})
SET(PR_Entity_Hdr
  ${PR_LIB}/entity/BoundaryEntity.h
  ${PR_LIB}/entity/Entity.h
  ${PR_LIB}/entity/PlaneEntity.h
  ${PR_LIB}/entity/MeshEntity.h
  ${PR_LIB}/entity/RenderEntity.h
  ${PR_LIB}/entity/SphereEntity.h)
SOURCE_GROUP("Header Files\\Entity" FILES ${PR_Entity_Hdr})
SET(PR_Geometry_Hdr
  ${PR_LIB}/geometry/BoundingBox.h
  ${PR_LIB}/geometry/Face.h
  ${PR_LIB}/geometry/FacePoint.h
  ${PR_LIB}/geometry/Mesh.h
  ${PR_LIB}/geometry/Plane.h
  ${PR_LIB}/geometry/Sphere.h
  ${PR_LIB}/geometry/Triangle.h)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_Lights_Hdr
  ${PR_LIB}/lights/PointLight.h)
SOURCE_GROUP("Header Files\\Lights" FILES ${PR_Lights_Hdr})
SET(PR_Material_Hdr
  ${PR_LIB}/material/BRDF.h
  ${PR_LIB}/material/BRDFMaterial.h
  ${PR_LIB}/material/DebugBoundingBoxMaterial.h
  ${PR_LIB}/material/DirectLightDebugMaterial.h
  ${PR_LIB}/material/GlassMaterial.h
  ${PR_LIB}/material/GridMaterial.h
  ${PR_LIB}/material/Material.h
  ${PR_LIB}/material/MirrorMaterial.h
  ${PR_LIB}/material/NormalDebugMaterial.h
  ${PR_LIB}/material/UVDebugMaterial.h)
SOURCE_GROUP("Header Files\\Material" FILES ${PR_Material_Hdr})
SET(PR_Photon_Hdr
  ${PR_LIB}/photon/Photon.h
  ${PR_LIB}/photon/PhotonMap.h)
SOURCE_GROUP("Header Files\\Photon" FILES ${PR_Photon_Hdr})
SET(PR_Ray_Hdr
  ${PR_LIB}/ray/Ray.h
  ${PR_LIB}/ray/Ray.inl)
SOURCE_GROUP("Header Files\\Ray" FILES ${PR_Ray_Hdr})
SET(PR_Renderer_Hdr
  ${PR_LIB}/renderer/RenderResult.h
  ${PR_LIB}/renderer/Renderer.h
  ${PR_LIB}/renderer/RenderThread.h)
SOURCE_GROUP("Header Files\\Renderer" FILES ${PR_Renderer_Hdr})
SET(PR_Sampler_Hdr
  ${PR_LIB}/sampler/Projection.h
  ${PR_LIB}/sampler/Sampler.h
  ${PR_LIB}/sampler/Stratified1DSampler.h
  ${PR_LIB}/sampler/Stratified2DSampler.h
  ${PR_LIB}/sampler/Stratified3DSampler.h)
SOURCE_GROUP("Header Files\\Sampler" FILES ${PR_Sampler_Hdr})
SET(PR_Scene_Hdr
  ${PR_LIB}/scene/kdTree.h
  ${PR_LIB}/scene/Scene.h)
SOURCE_GROUP("Header Files\\Scene" FILES ${PR_Scene_Hdr})
SET(PR_Spectral_Hdr
  ${PR_LIB}/spectral/IntensityConverter.h
  ${PR_LIB}/spectral/RGBConverter.h
  ${PR_LIB}/spectral/Spectrum.h
  ${PR_LIB}/spectral/Spectrum.inl
  ${PR_LIB}/spectral/XYZConverter.h)
SOURCE_GROUP("Header Files\\Spectral" FILES ${PR_Spectral_Hdr})
SET(PR_Thread_Hdr
  ${PR_LIB}/thread/Thread.h
  ${PR_LIB}/thread/Thread.inl)
SOURCE_GROUP("Header Files\\Thread" FILES ${PR_Thread_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Geometry_Hdr} ${PR_Loader_Hdr}
  ${PR_Entity_Hdr} ${PR_Ray_Hdr} ${PR_Scene_Hdr}
  ${PR_Renderer_Hdr} ${PR_Spectral_Hdr} ${PR_Material_Hdr}
  ${PR_Thread_Hdr} ${PR_Camera_Hdr} ${PR_Lights_Hdr} ${PR_Sampler_Hdr}
  ${PR_Photon_Hdr}
)
	
add_library(${PR_TARGET} SHARED ${PR_Src} ${PR_Hdr})
set_target_properties(${PR_TARGET} PROPERTIES VERSION ${PR_Version})
set_target_properties(${PR_TARGET} PROPERTIES COMPILE_DEFINITIONS "${DEFS};PR_LIB_BUILD;" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")