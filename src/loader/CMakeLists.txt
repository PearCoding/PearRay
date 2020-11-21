set(PR_Src
  DebugIO.cpp
  DebugIO.h
  DefaultSRGB.cpp
  DefaultSRGB.h
  ResourceManager.cpp
  ResourceManager.h
  Environment.cpp
  Environment.h
  Environment.inl
  ImageIO.cpp
  ImageIO.h
  QueryEnvironment.cpp
  QueryEnvironment.h
  QueryEnvironment.inl
  SceneLoadContext.cpp
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
  shader/ConstNode.cpp
  shader/ConstNode.h
  shader/EquidistantSpectrumNode.h
  shader/ImageNode.cpp
  shader/ImageNode.h
  shader/INodePlugin.h
  shader/NodeManager.cpp
  shader/NodeManager.h
  spectral/ISpectralMapperPlugin.h
  spectral/SpectralMapperManager.cpp
  spectral/SpectralMapperManager.h
  )

if(NOT WIN32)
  # For non Windows (prob. only Linux) systems we use assembler to embed srgb.coeff
  enable_language(ASM)
  set(RC embed/srgb.s)
endif()

add_library(pr_lib_loader ${PR_Src} ${RC})

if(WIN32)
  # This is not a satisfying solution, as srgb.coeff is quite large and each exe requires one build.
  # A solution would be to use a custom dll, but this would enforce the dll to ship along...
  # and then we could just ship srgb.coeff along...
  target_sources(pr_lib_loader PUBLIC embed/srgb.rc )
endif()
target_link_libraries(pr_lib_loader
  PUBLIC
    pr_lib_core
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

install(TARGETS pr_lib_loader COMPONENT runtime)
installHeaderFiles(${PR_Src})