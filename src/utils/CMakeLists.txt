set(PR_Src
  ResourceManager.cpp
  ResourceManager.h
  Environment.cpp
  Environment.h
  Environment.inl
  SceneLoadContext.h
  SceneLoader.cpp
  SceneLoader.h
  SpectralFile.cpp
  SpectralFile.h
  camera/CameraManager.cpp
  camera/CameraManager.h
  entity/EntityManager.cpp
  entity/EntityManager.h
  entity/IEntityPlugin.h
  emission/IEmissionPlugin.h
  emission/EmissionManager.cpp
  emission/EmissionManager.h
  filter/IFilterPlugin.h
  filter/FilterManager.cpp
  filter/FilterManager.h
  infinitelight/IInfiniteLightPlugin.h
  infinitelight/InfiniteLightManager.cpp
  infinitelight/InfiniteLightManager.h
  integrator/IIntegratorPlugin.h
  integrator/IntegratorManager.cpp
  integrator/IntegratorManager.h
  loader/PlyLoader.cpp
  loader/PlyLoader.h
  loader/SubGraphLoader.h
  loader/WavefrontLoader.cpp
  loader/WavefrontLoader.h
  material/IMaterialPlugin.h
  material/MaterialManager.cpp
  material/MaterialManager.h
  parameter/Parameter.cpp
  parameter/Parameter.h
  parameter/Parameter.inl
  parameter/ParameterGroup.h
  parameter/ParameterGroup.inl
  parser/CurveParser.cpp
  parser/CurveParser.h
  parser/MathParser.cpp
  parser/MathParser.h
  parser/MeshParser.cpp
  parser/MeshParser.h
  parser/SocketParser.cpp
  parser/SocketParser.h
  parser/SpectralParser.cpp
  parser/SpectralParser.h
  parser/TextureParser.cpp
  parser/TextureParser.h
  plugin/AbstractManager.h
  plugin/Plugin.h
  plugin/PluginManager.cpp
  plugin/PluginManager.h
  output/ImageWriter.cpp
  output/ImageWriter.h
  output/OutputSpecification.cpp
  output/OutputSpecification.h
  sampler/ISamplerPlugin.h
  sampler/SamplerManager.cpp
  sampler/SamplerManager.h
  shader/ConstSocket.cpp
  shader/ConstSocket.h
  shader/ImageMapSocket.cpp
  shader/ImageMapSocket.h
  shader/MapShadingSocket.cpp
  shader/MapShadingSocket.h)

add_library(pr_lib_utils ${PR_Src})
target_link_libraries(pr_lib_utils
  PUBLIC
    pr_lib Boost::iostreams Boost::regex Boost::filesystem
  PRIVATE
    datalisp OpenImageIO::Core OpenImageIO::Util
)
set_target_properties(pr_lib_utils PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_include_directories(pr_lib_utils PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(pr_lib_utils PUBLIC "PR_LIB_UTILS_STATIC")
  set_target_properties(pr_lib_utils PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
target_compile_definitions(pr_lib_utils PRIVATE "PR_LIB_UTILS_BUILD")

if(PR_COMPRESS_SPEC_FILES)
target_compile_definitions(pr_lib_utils PRIVATE "PR_COMPRESS_SPEC_FILES")
endif()

include(SetupEmbeddedPlugins)
add_embedded_plugins(pr_lib_utils)
add_lto(pr_lib_utils)
if(BUILD_SHARED_LIBS)
  strip_binary(pr_lib_utils)
endif()