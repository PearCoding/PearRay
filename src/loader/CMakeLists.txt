set(PR_Src
  DefaultSRGB.cpp
  DefaultSRGB.h
  ResourceManager.cpp
  ResourceManager.h
  Environment.cpp
  Environment.h
  Environment.inl
  SceneLoadContext.h
  SceneLoader.cpp
  SceneLoader.h
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
  archives/PlyLoader.cpp
  archives/PlyLoader.h
  archives/SubGraphLoader.h
  archives/WavefrontLoader.cpp
  archives/WavefrontLoader.h
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

if(NOT WIN32)
  enable_language(ASM)
  set(RC embed/srgb.s)
endif()

add_library(pr_lib_loader ${PR_Src} ${RC})
if(NOT WIN32)
  target_link_libraries(pr_lib_loader PUBLIC ${CMAKE_DL_LIBS})
endif()
target_link_libraries(pr_lib_loader
  PUBLIC
    pr_lib_core Boost::iostreams Boost::regex
  PRIVATE
    datalisp OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
)
set_target_properties(pr_lib_loader PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_include_directories(pr_lib_loader PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> PRIVATE ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(pr_lib_loader PUBLIC "PR_LIB_LOADER_STATIC")
  set_target_properties(pr_lib_loader PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
target_compile_definitions(pr_lib_loader PRIVATE "PR_LIB_LOADER_BUILD")

if(PR_COMPRESS_SPEC_FILES)
target_compile_definitions(pr_lib_loader PRIVATE "PR_COMPRESS_SPEC_FILES")
endif()

include(SetupEmbeddedPlugins)
add_embedded_plugins(pr_lib_loader)
add_lto(pr_lib_loader)
if(BUILD_SHARED_LIBS)
  strip_binary(pr_lib_loader)
endif()