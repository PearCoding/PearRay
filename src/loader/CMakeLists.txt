set(PR_Src
  DebugIO.cpp
  DebugIO.h
  DefaultSRGB.cpp
  DefaultSRGB.h
  Environment.cpp
  Environment.h
  Environment.inl
  ImageIO.cpp
  ImageIO.h
  ResourceManager.cpp
  ResourceManager.h
  SceneLoadContext.cpp
  SceneLoadContext.h
  SceneLoader.cpp
  SceneLoader.h
  archives/MtsSerializedLoader.cpp
  archives/MtsSerializedLoader.h
  archives/PlyLoader.cpp
  archives/PlyLoader.h
  archives/SubGraphLoader.h
  archives/WavefrontLoader.cpp
  archives/WavefrontLoader.h
  camera/CameraManager.cpp
  camera/CameraManager.h
  emission/EmissionManager.cpp
  emission/EmissionManager.h
  emission/IEmissionPlugin.h
  entity/EntityManager.cpp
  entity/EntityManager.h
  entity/IEntityPlugin.h
  filter/FilterManager.cpp
  filter/FilterManager.h
  filter/IFilterPlugin.h
  infinitelight/IInfiniteLightPlugin.h
  infinitelight/InfiniteLightManager.cpp
  infinitelight/InfiniteLightManager.h
  integrator/IIntegratorPlugin.h
  integrator/IntegratorManager.cpp
  integrator/IntegratorManager.h
  material/IMaterialPlugin.h
  material/MaterialManager.cpp
  material/MaterialManager.h
  output/FrameContainer.cpp
  output/FrameContainer.h
  output/FrameContainer.inl
  output/FrameOutputDevice.cpp
  output/FrameOutputDevice.h
  output/LocalFrameOutputDevice.cpp
  output/LocalFrameOutputDevice.h
  output/io/ImageWriter.cpp
  output/io/ImageWriter.h
  output/io/OutputSpecification.cpp
  output/io/OutputSpecification.h
  parameter/Parameter.cpp
  parameter/Parameter.h
  parameter/Parameter.inl
  parameter/ParameterDesc.h
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
  plugin/PluginSpecification.h
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
else()
  # This is not a satisfying solution, as srgb.coeff is quite large and each exe requires one build.
  # A solution would be to use a custom dll, but this would enforce the dll to ship along...
  # and then we could just ship srgb.coeff along...
  enable_language(RC)    
  set_source_files_properties(embed/srgb.rc LANGUAGE RC)
  set(RC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/embed/srgb.rc>)
  #add_library(pr_lib_win32_resources SHARED embed/srgb.rc)
  #set_target_properties(pr_lib_win32_resources PROPERTIES EXPORT_NAME resources)
  #install(TARGETS pr_lib_win32_resources COMPONENT runtime EXPORT PearRay-targets)
endif()

add_library(pr_lib_loader ${PR_Src} ${RC})

target_link_libraries(pr_lib_loader
  PUBLIC
    pr_lib_core
  PRIVATE
    ZLIB::ZLIB datalisp OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util
)

if(WIN32)
  target_sources(pr_lib_loader PUBLIC ${RC} )
  #target_link_libraries(pr_lib_loader PRIVATE pr_lib_win32_resources)
endif()

if(WIN32)
  target_link_libraries(pr_lib_loader PRIVATE OpenEXR::IlmImf)
endif()

set_target_properties(pr_lib_loader PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_include_directories(pr_lib_loader 
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_PREFIX}/>
  PRIVATE
    ${OpenEXR_INCLUDE_DIRS})# For lmathVec.h
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

target_setup_deploy(pr_lib_loader RECURSIVE)
if(BUILD_SHARED_LIBS)
  set(INSTALL_COMPONENT runtime)
else()
  set(INSTALL_COMPONENT development)
endif()
set_target_properties(pr_lib_loader PROPERTIES EXPORT_NAME loader)
install(TARGETS pr_lib_loader COMPONENT ${INSTALL_COMPONENT} EXPORT PearRay-targets)
installHeaderFiles(${PR_Src})