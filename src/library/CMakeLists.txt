CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_LIB ${PR_Root}/library)

SET(PR_Main_Src
  ${PR_LIB}/FileLogListener.cpp
  ${PR_LIB}/Logger.cpp)
SET(PR_Affector_Src
  ${PR_LIB}/affector/LightAffector.cpp
  ${PR_LIB}/affector/PhotonAffector.cpp)
SOURCE_GROUP("Source Files\\Affector" FILES ${PR_Affector_Src})
SET(PR_Camera_Src
  ${PR_LIB}/camera/Camera.cpp
  ${PR_LIB}/camera/StandardCamera.cpp)
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
  ${PR_LIB}/geometry/Plane.cpp
  ${PR_LIB}/geometry/Sphere.cpp
  ${PR_LIB}/geometry/TriMesh.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_GPU_Src
  ${PR_LIB}/gpu/GPU.cpp)
SOURCE_GROUP("Source Files\\GPU" FILES ${PR_GPU_Src})
SET(PR_Integrator_Src
  ${PR_LIB}/integrator/BiDirectIntegrator.cpp
  ${PR_LIB}/integrator/DebugIntegrator.cpp
  ${PR_LIB}/integrator/DirectIntegrator.cpp)
SOURCE_GROUP("Source Files\\Integrator" FILES ${PR_Integrator_Src})
SET(PR_Material_Src
  ${PR_LIB}/material/BlinnPhongMaterial.cpp
  ${PR_LIB}/material/BRDF.cpp
  ${PR_LIB}/material/DebugBoundingBoxMaterial.cpp
  ${PR_LIB}/material/DiffuseMaterial.cpp
  ${PR_LIB}/material/GlassMaterial.cpp
  ${PR_LIB}/material/GridMaterial.cpp
  ${PR_LIB}/material/Material.cpp
  ${PR_LIB}/material/MirrorMaterial.cpp
  ${PR_LIB}/material/OrenNayarMaterial.cpp
  ${PR_LIB}/material/WardMaterial.cpp)
SOURCE_GROUP("Source Files\\Material" FILES ${PR_Material_Src})
SET(PR_Math_Src ) 
SOURCE_GROUP("Source Files\\Math" FILES ${PR_Math_Src})
SET(PR_Photon_Src
  ${PR_LIB}/photon/PhotonMap.cpp)
SOURCE_GROUP("Source Files\\Photon" FILES ${PR_Photon_Src})
SET(PR_Ray_Src
  ${PR_LIB}/ray/Ray.cpp)
SOURCE_GROUP("Source Files\\Ray" FILES ${PR_Ray_Src})
SET(PR_Renderer_Src
  ${PR_LIB}/renderer/Renderer.cpp
  ${PR_LIB}/renderer/RenderContext.cpp
  ${PR_LIB}/renderer/RenderResult.cpp
  ${PR_LIB}/renderer/RenderSettings.cpp
  ${PR_LIB}/renderer/RenderTile.cpp
  ${PR_LIB}/renderer/RenderThread.cpp)
SOURCE_GROUP("Source Files\\Renderer" FILES ${PR_Renderer_Src})
SET(PR_Sampler_Src
  ${PR_LIB}/sampler/MultiJitteredSampler.cpp
  ${PR_LIB}/sampler/RandomSampler.cpp
  ${PR_LIB}/sampler/StratifiedSampler.cpp
  ${PR_LIB}/sampler/UniformSampler.cpp)
SOURCE_GROUP("Source Files\\Sampler" FILES ${PR_Sampler_Src})
SET(PR_Scene_Src
  ${PR_LIB}/scene/Scene.cpp)
SOURCE_GROUP("Source Files\\Scene" FILES ${PR_Scene_Src})
SET(PR_Spectral_Src
  ${PR_LIB}/spectral/IntensityConverter.cpp
  ${PR_LIB}/spectral/RGBConverter.cpp
  ${PR_LIB}/spectral/Spectrum.cpp
  ${PR_LIB}/spectral/ToneMapper.cpp
  ${PR_LIB}/spectral/XYZConverter.cpp)
SOURCE_GROUP("Source Files\\Spectral" FILES ${PR_Spectral_Src})
SET(PR_Shader_Src
)
SOURCE_GROUP("Source Files\\Shader" FILES ${PR_Shader_Src})
SET(PR_Thread_Src
  ${PR_LIB}/thread/Thread.cpp)
SOURCE_GROUP("Source Files\\Thread" FILES ${PR_Thread_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
  ${PR_Entity_Src} ${PR_Ray_Src} ${PR_Scene_Src}
  ${PR_Renderer_Src} ${PR_Spectral_Src} ${PR_Material_Src}
  ${PR_Thread_Src} ${PR_Camera_Src} ${PR_Sampler_Src}
  ${PR_Photon_Src} ${PR_Integrator_Src} ${PR_Shader_Src} 
  ${PR_Affector_Src} ${PR_Math_Src} ${PR_GPU_Src}
)

SET(PR_Main_Hdr
  ${PR_LIB}/Config.h.in
  ${PR_LIB}/FileLogListener.h
  ${PR_LIB}/Logger.h
  ${PR_LIB}/LogListener.h
  ${PR_LIB}/Random.h)
SET(PR_Affector_Hdr
  ${PR_LIB}/affector/Affector.h
  ${PR_LIB}/affector/LightAffector.h
  ${PR_LIB}/affector/PhotonAffector.h)
SOURCE_GROUP("Header Files\\Affector" FILES ${PR_Affector_Hdr})
SET(PR_Camera_Hdr
  ${PR_LIB}/camera/Camera.h
  ${PR_LIB}/camera/StandardCamera.h)
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
  ${PR_LIB}/geometry/IMesh.h
  ${PR_LIB}/geometry/Plane.h
  ${PR_LIB}/geometry/Sphere.h
  ${PR_LIB}/geometry/Triangle.h
  ${PR_LIB}/geometry/TriMesh.h)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_GPU_Hdr
  ${PR_LIB}/gpu/GPU.h)
SOURCE_GROUP("Header Files\\GPU" FILES ${PR_GPU_Hdr})
SET(PR_Integrator_Hdr
  ${PR_LIB}/integrator/BiDirectIntegrator.h
  ${PR_LIB}/integrator/DebugIntegrator.h
  ${PR_LIB}/integrator/DirectIntegrator.h
  ${PR_LIB}/integrator/Integrator.h)
SOURCE_GROUP("Header Files\\Integrator" FILES ${PR_Integrator_Hdr})
SET(PR_Material_Hdr 
  ${PR_LIB}/material/BlinnPhongMaterial.h
  ${PR_LIB}/material/BRDF.h
  ${PR_LIB}/material/DebugBoundingBoxMaterial.h
  ${PR_LIB}/material/DiffuseMaterial.h
  ${PR_LIB}/material/GlassMaterial.h
  ${PR_LIB}/material/GridMaterial.h
  ${PR_LIB}/material/Material.h
  ${PR_LIB}/material/MirrorMaterial.h
  ${PR_LIB}/material/OrenNayarMaterial.h
  ${PR_LIB}/material/WardMaterial.h)
SOURCE_GROUP("Header Files\\Material" FILES ${PR_Material_Hdr})
SET(PR_Math_Hdr
  ${PR_LIB}/math/Fresnel.h
  ${PR_LIB}/math/MSI.h
  ${PR_LIB}/math/Projection.h
  ${PR_LIB}/math/Reflection.h)
SOURCE_GROUP("Header Files\\Math" FILES ${PR_Math_Hdr})
SET(PR_Photon_Hdr
  ${PR_LIB}/photon/Photon.h
  ${PR_LIB}/photon/PhotonMap.h)
SOURCE_GROUP("Header Files\\Photon" FILES ${PR_Photon_Hdr})
SET(PR_Ray_Hdr
  ${PR_LIB}/ray/Ray.h
  ${PR_LIB}/ray/Ray.inl)
SOURCE_GROUP("Header Files\\Ray" FILES ${PR_Ray_Hdr})
SET(PR_Renderer_Hdr
  ${PR_LIB}/renderer/Renderer.h
  ${PR_LIB}/renderer/RenderContext.h
  ${PR_LIB}/renderer/RenderResult.h
  ${PR_LIB}/renderer/RenderSettings.h
  ${PR_LIB}/renderer/RenderTile.h
  ${PR_LIB}/renderer/RenderThread.h)
SOURCE_GROUP("Header Files\\Renderer" FILES ${PR_Renderer_Hdr})
SET(PR_Sampler_Hdr
  ${PR_LIB}/sampler/MultiJitteredSampler.h
  ${PR_LIB}/sampler/RandomSampler.h
  ${PR_LIB}/sampler/Sampler.h
  ${PR_LIB}/sampler/StratifiedSampler.h
  ${PR_LIB}/sampler/UniformSampler.h)
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
  ${PR_LIB}/spectral/ToneMapper.h
  ${PR_LIB}/spectral/XYZConverter.h)
SOURCE_GROUP("Header Files\\Spectral" FILES ${PR_Spectral_Hdr})
SET(PR_Shader_Hdr
  ${PR_LIB}/shader/ShaderOutput.h
  ${PR_LIB}/shader/SamplePoint.h)
SOURCE_GROUP("Header Files\\Shader" FILES ${PR_Shader_Hdr})
SET(PR_Thread_Hdr
  ${PR_LIB}/thread/Thread.h
  ${PR_LIB}/thread/Thread.inl)
SOURCE_GROUP("Header Files\\Thread" FILES ${PR_Thread_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Geometry_Hdr} ${PR_Loader_Hdr}
  ${PR_Entity_Hdr} ${PR_Ray_Hdr} ${PR_Scene_Hdr}
  ${PR_Renderer_Hdr} ${PR_Spectral_Hdr} ${PR_Material_Hdr}
  ${PR_Thread_Hdr} ${PR_Camera_Hdr} ${PR_Sampler_Hdr}
  ${PR_Photon_Hdr} ${PR_Integrator_Hdr} ${PR_Shader_Hdr}
  ${PR_Affector_Hdr} ${PR_Math_Hdr} ${PR_GPU_Hdr}
)

SET(PR_CL
  ${PR_LIB}/cl/tonemapper.cl
  ${PR_LIB}/cl/xyztable.cl)
SOURCE_GROUP("CL Files" FILES ${PR_CL})
  	
add_library(${PR_TARGET} SHARED ${PR_Src} ${PR_Hdr} ${PR_CL})
target_link_libraries(${PR_TARGET} ${Boost_LIBRARIES})

IF(OPENCL_FOUND AND NOT PR_NO_GPU)
	target_link_libraries(${PR_TARGET} ${OpenCL_LIBRARIES})
ENDIF()

set_target_properties(${PR_TARGET} PROPERTIES VERSION ${PR_Version})
target_compile_definitions(${PR_TARGET} PRIVATE "${DEFS};PR_LIB_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")