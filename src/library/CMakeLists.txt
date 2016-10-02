CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  FileLogListener.cpp
  Logger.cpp)
SET(PR_Camera_Src
  camera/Camera.cpp
  camera/StandardCamera.cpp)
SOURCE_GROUP("Source Files\\Camera" FILES ${PR_Camera_Src})
SET(PR_Entity_Src
  entity/BoundaryEntity.cpp
  entity/Entity.cpp
  entity/MeshEntity.cpp
  entity/PlaneEntity.cpp
  entity/RenderEntity.cpp
  entity/SphereEntity.cpp)
SOURCE_GROUP("Source Files\\Entity" FILES ${PR_Entity_Src})
SET(PR_Geometry_Src
  geometry/BoundingBox.cpp
  geometry/Plane.cpp
  geometry/Sphere.cpp
  geometry/TriMesh.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_GPU_Src
  gpu/GPU.cpp)
SOURCE_GROUP("Source Files\\GPU" FILES ${PR_GPU_Src})
SET(PR_Integrator_Src
  integrator/BiDirectIntegrator.cpp
  integrator/DebugIntegrator.cpp
  integrator/DirectIntegrator.cpp
  integrator/Integrator.cpp
  integrator/OnePassIntegrator.cpp
  integrator/PPMIntegrator.cpp)
SOURCE_GROUP("Source Files\\Integrator" FILES ${PR_Integrator_Src})
SET(PR_Light_Src
  light/DistantLight.cpp
  light/EnvironmentLight.cpp)
SOURCE_GROUP("Source Files\\Light" FILES ${PR_Light_Src})
SET(PR_Material_Src
  material/BlinnPhongMaterial.cpp
  material/DiffuseMaterial.cpp
  material/GlassMaterial.cpp
  material/GridMaterial.cpp
  material/Material.cpp
  material/MirrorMaterial.cpp
  material/OrenNayarMaterial.cpp
  material/WardMaterial.cpp)
SOURCE_GROUP("Source Files\\Material" FILES ${PR_Material_Src})
SET(PR_Math_Src ) 
SOURCE_GROUP("Source Files\\Math" FILES ${PR_Math_Src})
SET(PR_Performance_Src
  performance/PerformanceWriter.cpp)
SOURCE_GROUP("Source Files\\Performance" FILES ${PR_Performance_Src})
SET(PR_Photon_Src )
SOURCE_GROUP("Source Files\\Photon" FILES ${PR_Photon_Src})
SET(PR_Ray_Src
  ray/Ray.cpp)
SOURCE_GROUP("Source Files\\Ray" FILES ${PR_Ray_Src})
SET(PR_Renderer_Src
  renderer/PixelMap.cpp
  renderer/PPMSettings.cpp
  renderer/Renderer.cpp
  renderer/RenderContext.cpp
  renderer/RenderSettings.cpp
  renderer/RenderStatistics.cpp
  renderer/RenderTile.cpp
  renderer/RenderThread.cpp)
SOURCE_GROUP("Source Files\\Renderer" FILES ${PR_Renderer_Src})
SET(PR_Sampler_Src
  sampler/HaltonQMCSampler.cpp
  sampler/MultiJitteredSampler.cpp
  sampler/RandomSampler.cpp
  sampler/StratifiedSampler.cpp
  sampler/UniformSampler.cpp)
SOURCE_GROUP("Source Files\\Sampler" FILES ${PR_Sampler_Src})
SET(PR_Scene_Src
  scene/Scene.cpp)
SOURCE_GROUP("Source Files\\Scene" FILES ${PR_Scene_Src})
SET(PR_Spectral_Src
  spectral/IntensityConverter.cpp
  spectral/RGBConverter.cpp
  spectral/Spectrum.cpp
  spectral/ToneMapper.cpp
  spectral/XYZConverter.cpp)
SOURCE_GROUP("Source Files\\Spectral" FILES ${PR_Spectral_Src})
SET(PR_Shader_Src
)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Thread_Src
  thread/Thread.cpp)
SOURCE_GROUP("Source Files\\Thread" FILES ${PR_Thread_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
  ${PR_Entity_Src} ${PR_Ray_Src} ${PR_Scene_Src}
  ${PR_Renderer_Src} ${PR_Spectral_Src} ${PR_Material_Src}
  ${PR_Thread_Src} ${PR_Camera_Src} ${PR_Sampler_Src}
  ${PR_Photon_Src} ${PR_Integrator_Src} ${PR_Shader_Src} 
  ${PR_Affector_Src} ${PR_Math_Src} ${PR_GPU_Src} 
  ${PR_Performance_Src} ${PR_Light_Src}
)

SET(PR_Main_Hdr
  Config.h.in
  FileLogListener.h
  Logger.h
  LogListener.h
  Random.h)
SET(PR_Camera_Hdr
  camera/Camera.h
  camera/StandardCamera.h)
SOURCE_GROUP("Header Files\\Camera" FILES ${PR_Camera_Hdr})
SET(PR_Entity_Hdr
  entity/BoundaryEntity.h
  entity/Entity.h
  entity/Entity.inl
  entity/PlaneEntity.h
  entity/MeshEntity.h
  entity/RenderEntity.h
  entity/SphereEntity.h)
SOURCE_GROUP("Header Files\\Entity" FILES ${PR_Entity_Hdr})
SET(PR_Geometry_Hdr
  geometry/BoundingBox.h
  geometry/Face.h
  geometry/IMesh.h
  geometry/Plane.h
  geometry/Sphere.h
  geometry/Triangle.h
  geometry/TriMesh.h)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_GPU_Hdr
  gpu/GPU.h)
SOURCE_GROUP("Header Files\\GPU" FILES ${PR_GPU_Hdr})
SET(PR_Integrator_Hdr
  integrator/BiDirectIntegrator.h
  integrator/DebugIntegrator.h
  integrator/DirectIntegrator.h
  integrator/Integrator.h
  integrator/OnePassIntegrator.h
  integrator/PPMIntegrator.h)
SOURCE_GROUP("Header Files\\Integrator" FILES ${PR_Integrator_Hdr})
SET(PR_Light_Hdr
  light/DistantLight.h
  light/EnvironmentLight.h
  light/IInfiniteLight.h)
SOURCE_GROUP("Header Files\\Light" FILES ${PR_Light_Hdr})
SET(PR_Material_Hdr 
  material/BlinnPhongMaterial.h
  material/BRDF.h
  material/BRDF.inl
  material/DiffuseMaterial.h
  material/GlassMaterial.h
  material/GridMaterial.h
  material/Material.h
  material/MirrorMaterial.h
  material/OrenNayarMaterial.h
  material/WardMaterial.h)
SOURCE_GROUP("Header Files\\Material" FILES ${PR_Material_Hdr})
SET(PR_Math_Hdr
  math/Fresnel.h
  math/HemiMap.h
  math/MSI.h
  math/Projection.h
  math/ProjectionMap.h
  math/Reflection.h
  math/SphereMap.h)
SOURCE_GROUP("Header Files\\Math" FILES ${PR_Math_Hdr})
SET(PR_Performance_Hdr
  performance/Performance.h
  performance/Performance.inl
  performance/PerformanceWriter.h)
SOURCE_GROUP("Header Files\\Performance" FILES ${PR_Performance_Hdr})
SET(PR_Photon_Hdr
  photon/Photon.h
  photon/PointMap.h
  photon/PointMap.inl)
SOURCE_GROUP("Header Files\\Photon" FILES ${PR_Photon_Hdr})
SET(PR_Ray_Hdr
  ray/Ray.h
  ray/Ray.inl)
SOURCE_GROUP("Header Files\\Ray" FILES ${PR_Ray_Hdr})
SET(PR_Renderer_Hdr
  renderer/DisplayDriver.h
  renderer/PixelMap.h
  renderer/PPMSettings.h
  renderer/Renderer.h
  renderer/RenderContext.h
  renderer/RenderSettings.h
  renderer/RenderStatistics.h
  renderer/RenderTile.h
  renderer/RenderThread.h)
SOURCE_GROUP("Header Files\\Renderer" FILES ${PR_Renderer_Hdr})
SET(PR_Sampler_Hdr
  sampler/HaltonQMCSampler.h
  sampler/MultiJitteredSampler.h
  sampler/RandomSampler.h
  sampler/Sampler.h
  sampler/StratifiedSampler.h
  sampler/UniformSampler.h)
SOURCE_GROUP("Header Files\\Sampler" FILES ${PR_Sampler_Hdr})
SET(PR_Scene_Hdr
  scene/kdTree.h
  scene/Scene.h)
SOURCE_GROUP("Header Files\\Scene" FILES ${PR_Scene_Hdr})
SET(PR_Spectral_Hdr
  spectral/IntensityConverter.h
  spectral/RGBConverter.h
  spectral/Spectrum.h
  spectral/Spectrum.inl
  spectral/ToneMapper.h
  spectral/XYZConverter.h)
SOURCE_GROUP("Header Files\\Spectral" FILES ${PR_Spectral_Hdr})
SET(PR_Shader_Hdr
  shader/FaceSample.h
  shader/ShaderClosure.h
  shader/ShaderOutput.h)
SOURCE_GROUP("Header Files\\Shader" FILES ${PR_Shader_Hdr})
SET(PR_Thread_Hdr
  thread/Thread.h
  thread/Thread.inl)
SOURCE_GROUP("Header Files\\Thread" FILES ${PR_Thread_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Geometry_Hdr} ${PR_Loader_Hdr}
  ${PR_Entity_Hdr} ${PR_Ray_Hdr} ${PR_Scene_Hdr}
  ${PR_Renderer_Hdr} ${PR_Spectral_Hdr} ${PR_Material_Hdr}
  ${PR_Thread_Hdr} ${PR_Camera_Hdr} ${PR_Sampler_Hdr}
  ${PR_Photon_Hdr} ${PR_Integrator_Hdr} ${PR_Shader_Hdr}
  ${PR_Affector_Hdr} ${PR_Math_Hdr} ${PR_GPU_Hdr}
  ${PR_Performance_Hdr} ${PR_Light_Hdr}
)

SET(PR_CL
  cl/tonemapper.cl
  cl/xyztable.cl)
SOURCE_GROUP("CL Files" FILES ${PR_CL})
  	
add_library(pr_lib SHARED ${PR_Src} ${PR_Hdr} ${PR_CL})
target_link_libraries(pr_lib ${Boost_LIBRARIES})

IF(OPENCL_FOUND AND NOT PR_NO_GPU)
	target_link_libraries(pr_lib ${OpenCL_LIBRARIES})
ENDIF()

set_target_properties(pr_lib PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib PRIVATE "${DEFS};PR_LIB_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")