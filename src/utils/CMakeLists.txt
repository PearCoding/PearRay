CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Src
  Environment.cpp
  Environment.h
  Environment.inl
  SceneLoader.cpp
  SceneLoader.h
  SpectralFile.cpp
  SpectralFile.h
  camera/CameraManager.cpp
  camera/CameraManager.h
  entity/EntityManager.cpp
  entity/EntityManager.h
  entity/IEntityFactory.h
  emission/IEmissionFactory.h
  emission/EmissionManager.cpp
  emission/EmissionManager.h
  infinitelight/IInfiniteLightFactory.h
  infinitelight/InfiniteLightManager.cpp
  infinitelight/InfiniteLightManager.h
  integrator/IIntegratorFactory.h
  integrator/IntegratorManager.cpp
  integrator/IntegratorManager.h
  loader/SubGraphLoader.h
  loader/WavefrontLoader.cpp
  loader/WavefrontLoader.h
  material/IMaterialFactory.h
  material/MaterialManager.cpp
  material/MaterialManager.h
  parser/mesh/TriMeshInlineParser.cpp
  parser/mesh/TriMeshInlineParser.h
  parser/texture/TextureParser.cpp
  parser/texture/TextureParser.h
  plugin/AbstractManager.h
  plugin/Plugin.h
  plugin/PluginManager.cpp
  plugin/PluginManager.h
  output/ImageWriter.cpp
  output/ImageWriter.h
  output/OutputSpecification.cpp
  output/OutputSpecification.h
  registry/Any.h
  registry/Registry.cpp
  registry/Registry.h
  registry/Registry.inl
  registry/URI.cpp
  registry/URI.h
  registry/URI.inl
  shader/ConstShadingSocket.cpp
  shader/ConstShadingSocket.h
  shader/ImageShadingSocket.cpp
  shader/ImageShadingSocket.h)

add_library(pr_lib_utils SHARED ${PR_Src})
target_link_libraries(pr_lib_utils pr_lib)
target_link_libraries(pr_lib_utils ${DL_TARGET})
target_link_libraries(pr_lib_utils ${OIIO_LIBRARIES})

set_target_properties(pr_lib_utils PROPERTIES VERSION ${PR_Version})
set_target_properties(pr_lib_utils PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(pr_lib_utils PRIVATE "${DEFS};PR_LIB_UTILS_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")

IF(PR_CPP_CHECK)
  add_cpplint_target(pr_lib_utils_cppcheck "${PR_Src}")
ENDIF()