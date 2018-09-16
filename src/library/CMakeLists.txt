CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  CTP.cpp
  IFreezable.cpp
  FileLogListener.cpp
  Logger.cpp)
SET(PR_Buffer_Src
  buffer/ColorBuffer.cpp)
SOURCE_GROUP("Source Files\\Buffer" FILES ${PR_Buffer_Src})
SET(PR_Camera_Src
  camera/ICamera.cpp
  camera/CameraManager.cpp)
SOURCE_GROUP("Source Files\\Camera" FILES ${PR_Camera_Src})
SET(PR_Container_Src 
  container/kdTreeBuilder.cpp
  container/kdTreeBuilderNaive.cpp
  container/kdTreeCollider.cpp
  )
SOURCE_GROUP("Source Files\\Container" FILES ${PR_Container_Src})
SET(PR_Entity_Src
  entity/EntityManager.cpp
  entity/IEntity.cpp
  entity/VirtualEntity.cpp)
SOURCE_GROUP("Source Files\\Entity" FILES ${PR_Entity_Src})
SET(PR_Geometry_Src
  geometry/BoundingBox.cpp
  geometry/Plane.cpp
  geometry/Sphere.cpp
  geometry/TriMesh.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_Integrator_Src
  integrator/IIntegrator.cpp)
SOURCE_GROUP("Source Files\\Integrator" FILES ${PR_Integrator_Src})
SET(PR_InfiniteLight_Src
  light/ILight.cpp
  light/LightManager.cpp)
SOURCE_GROUP("Source Files\\InfiniteLight" FILES ${PR_InfiniteLight_Src})
SET(PR_Light_Src
  infinitelight/IInfiniteLight.cpp
  infinitelight/InfiniteLightManager.cpp)
SOURCE_GROUP("Source Files\\Light" FILES ${PR_Light_Src})
SET(PR_Material_Src
  material/IMaterial.cpp
  material/MaterialManager.cpp)
SOURCE_GROUP("Source Files\\Material" FILES ${PR_Material_Src})
SET(PR_Math_Src ) 
SOURCE_GROUP("Source Files\\Math" FILES ${PR_Math_Src})
SET(PR_Path_Src 
  path/LightPathExpression.cpp
  path/LightPathManager.cpp
  path/LightPath.cpp
  path/LPE_Automaton.cpp
  path/LPE_Parser.cpp
  path/LPE_RegExpr.cpp)
SOURCE_GROUP("Source Files\\Path" FILES ${PR_Path_Src})
SET(PR_Photon_Src )
SOURCE_GROUP("Source Files\\Photon" FILES ${PR_Photon_Src})
SET(PR_Plugin_Src 
  plugin/PluginLoader.cpp)
SOURCE_GROUP("Source Files\\Plugin" FILES ${PR_Plugin_Src})
SET(PR_Ray_Src
  ray/Ray.cpp
  ray/RayStream.cpp)
SOURCE_GROUP("Source Files\\Ray" FILES ${PR_Ray_Src})
SET(PR_Registry_Src
  registry/Registry.cpp
  registry/URI.cpp)
SOURCE_GROUP("Source Files\\Registry" FILES ${PR_Registry_Src})
SET(PR_Renderer_Src
  renderer/OutputMap.cpp
  renderer/RenderContext.cpp
  renderer/RenderFactory.cpp
  renderer/RenderManager.cpp
  renderer/RenderSession.cpp
  renderer/RenderSettings.cpp
  renderer/RenderStatistics.cpp
  renderer/RenderStatus.cpp
  renderer/RenderTile.cpp
  renderer/RenderTileMap.cpp
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
  spectral/SpectrumDescriptor.cpp
  spectral/ToneMapper.cpp
  spectral/XYZConverter.cpp)
SOURCE_GROUP("Source Files\\Spectral" FILES ${PR_Spectral_Src})
SET(PR_Shader_Src
  shader/ConstShadingSocket.cpp
  shader/HitStream.cpp)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Thread_Src
  thread/Thread.cpp)
SOURCE_GROUP("Source Files\\Thread" FILES ${PR_Thread_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
  ${PR_Entity_Src} ${PR_Ray_Src} ${PR_Scene_Src}
  ${PR_Renderer_Src} ${PR_Spectral_Src} ${PR_Material_Src}
  ${PR_Thread_Src} ${PR_Camera_Src} ${PR_Sampler_Src}
  ${PR_Photon_Src} ${PR_Integrator_Src} ${PR_Shader_Src} 
  ${PR_Affector_Src} ${PR_Math_Src} ${PR_Buffer_Src}
  ${PR_Light_Src} ${PR_Container_Src} ${PR_Registry_Src}
  ${PR_Plugin_Src} ${PR_InfiniteLight_Src} ${PR_Path_Src}
)

SET(PR_Main_Hdr
  CTP.h
  PR_Config.h.in
  FileLogListener.h
  IFreezable.h
  Logger.h
  LogListener.h
  Random.h
  Variant.h)
SET(PR_Buffer_Hdr
  buffer/ColorBuffer.h
  buffer/FrameBuffer.h)
SOURCE_GROUP("Header Files\\Buffer" FILES ${PR_Buffer_Hdr})
SET(PR_Camera_Hdr
  camera/ICamera.h
  camera/CameraManager.h)
SOURCE_GROUP("Header Files\\Camera" FILES ${PR_Camera_Hdr})
SET(PR_Container_Hdr
  container/kdTreeBuilder.h
  container/kdTreeBuilderNaive.h
  container/kdTreeCollider.h
  container/RadixSort.h)
SOURCE_GROUP("Header Files\\Container" FILES ${PR_Container_Hdr})
SET(PR_Entity_Hdr
  entity/EntityManager.h
  entity/IEntity.h
  entity/IEntity.inl
  entity/IEntityFactory.h
  entity/VirtualEntity.h
  entity/VirtualEntity.inl)
SOURCE_GROUP("Header Files\\Entity" FILES ${PR_Entity_Hdr}) 
SET(PR_Geometry_Hdr
  geometry/BoundingBox.h
  geometry/Face.h
  geometry/Plane.h
  geometry/Sphere.h
  geometry/Triangle.h
  geometry/TriMesh.h
  geometry/TriMesh.inl)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_Integrator_Hdr
  integrator/IIntegrator.h
  integrator/IIntegratorFactory.h)
SOURCE_GROUP("Header Files\\Integrator" FILES ${PR_Integrator_Hdr})
SET(PR_InfiniteLight_Hdr
  infinitelight/IInfiniteLight.h
  infinitelight/IInfiniteLightFactory.h
  infinitelight/InfiniteLightManager.h)
SOURCE_GROUP("Header Files\\InfiniteLight" FILES ${PR_InfiniteLight_Hdr})
SET(PR_Light_Hdr
  light/ILight.h
  light/ILightFactory.h
  light/LightManager.h)
SOURCE_GROUP("Header Files\\Light" FILES ${PR_Light_Hdr})
SET(PR_Material_Hdr 
  material/IMaterial.h
  material/IMaterial.inl
  material/IMaterialFactory.h
  material/MaterialManager.h)
SOURCE_GROUP("Header Files\\Material" FILES ${PR_Material_Hdr})
SET(PR_Math_Hdr
  math/Compression.h
  math/Fresnel.h
  math/Generator.h
  math/HemiMap.h
  math/MSI.h
  math/Projection.h
  math/ProjectionMap.h
  math/Reflection.h
  math/SIMath.h
  math/SIMathUtils.h
  math/SphereMap.h
  math/Tangent.h
  math/Vector.h)
SOURCE_GROUP("Header Files\\Math" FILES ${PR_Math_Hdr})
SET(PR_Path_Hdr
  path/LightPath.h
  path/LightPathExpression.h
  path/LightPathManager.h
  path/LightPathToken.h
  path/LPE_Automaton.h
  path/LPE_Parser.h
  path/LPE_RegExpr.h
  path/LPE_RegState.h)
SOURCE_GROUP("Header Files\\Path" FILES ${PR_Path_Hdr})
SET(PR_Photon_Hdr
  photon/Photon.h
  photon/PhotonMap.h
  photon/PhotonMap.inl)
SOURCE_GROUP("Header Files\\Photon" FILES ${PR_Photon_Hdr})
SET(PR_Plugin_Hdr
  plugin/Plugin.h
  plugin/PluginLoader.h)
SOURCE_GROUP("Header Files\\Plugin" FILES ${PR_Plugin_Hdr})
SET(PR_Ray_Hdr
  ray/Ray.h
  ray/Ray.inl
  ray/RayStream.h)
SOURCE_GROUP("Header Files\\Ray" FILES ${PR_Ray_Hdr})
SET(PR_Registry_Hdr
  registry/Any.h
  registry/Registry.h
  registry/Registry.inl
  registry/URI.h
  registry/URI.inl)
SOURCE_GROUP("Header Files\\Registry" FILES ${PR_Registry_Hdr})
SET(PR_Renderer_Hdr
  renderer/OutputMap.h
  renderer/RenderContext.h
  renderer/RenderEnums.h
  renderer/RenderFactory.h
  renderer/RenderManager.h
  renderer/RenderSession.h
  renderer/RenderSettings.h
  renderer/RenderStatistics.h
  renderer/RenderStatus.h
  renderer/RenderTile.h
  renderer/RenderTileMap.h
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
  scene/Scene.h)
SOURCE_GROUP("Header Files\\Scene" FILES ${PR_Scene_Hdr})
SET(PR_Spectral_Hdr
  spectral/IntensityConverter.h
  spectral/RGBConverter.h
  spectral/Spectrum.h
  spectral/Spectrum.inl
  spectral/SpectrumDescriptor.h
  spectral/SpectrumDescriptor.inl
  spectral/SpectrumLazyOperator.h
  spectral/ToneMapper.h
  spectral/xyz2spec.inl
  spectral/XYZConverter.h)
SOURCE_GROUP("Header Files\\Spectral" FILES ${PR_Spectral_Hdr})
SET(PR_Shader_Hdr
  shader/ConstShadingSocket.h
  shader/HitStream.h
  shader/ShadingGroup.h
  shader/ShadingPoint.h
  shader/ShadingSocket.h)
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
  ${PR_Affector_Hdr} ${PR_Math_Hdr} ${PR_Buffer_Hdr}
  ${PR_Light_Hdr} ${PR_Registry_Hdr} ${PR_Plugin_Hdr}
  ${PR_InfiniteLight_Hdr} ${PR_Path_Hdr}
)
  	
add_library(pr_lib SHARED ${PR_Src} ${PR_Hdr} ${PR_CL})
if(NOT WIN32)
  target_link_libraries(pr_lib pthread )
endif()
target_link_libraries(pr_lib ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})

set_target_properties(pr_lib PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_lib PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(pr_lib PRIVATE "${DEFS};PR_LIB_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")

IF(PR_CPP_CHECK)
add_cpplint_target(pr_lib_cppcheck "${PR_Src};${PR_Hdr}")
ENDIF()