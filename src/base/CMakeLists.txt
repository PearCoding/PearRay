set(PR_Src
  PR_Config.h.in
  Logger.cpp
  Logger.h
  PrettyPrint.h
  Platform.h
  Platform.cpp
  Profiler.h
  Profiler.cpp
  Variant.h
  arch/FileLock.cpp
  arch/FileLock.h
  arch/SharedLibrary.cpp
  arch/SharedLibrary.h
  config/Build.cpp
  config/Build.h
  config/Constants.inl
  config/HW_Switch.inl.in
  config/SIMD.inl
  config/Types.inl
  config/VectorGlue.inl
  config/Version.h.in
  container/IndexSort.h
  container/Interval.h
  container/NTree.h
  container/NTree.inl
  container/NTreeStackBuilder.h
  container/NTreeStackBuilder.inl
  container/QuickSort.h
  container/RadixSort.h
  curve/Curve.h
  log/ConsoleLogListener.cpp
  log/ConsoleLogListener.h
  log/FileLogListener.cpp
  log/FileLogListener.h
  log/LogListener.h
  network/Protocol.cpp
  network/Protocol.h
  network/Socket.cpp
  network/Socket.h
  network/SocketImplLinux.inl
  network/SocketImplWindows.inl
  math/Bits.h
  math/Compression.h
  math/Concentric.h
  math/Distribution1D.h
  math/Distribution1D.inl
  math/Fresnel.h
  math/Generator.h
  math/Hash.h
  math/ImportanceSampling.h
  math/Microfacet.h
  math/MicrofacetReflection.h
  math/MicrofacetTransmission.h
  math/Normal.h
  math/Projection.h
  math/ProjectionMap.h
  math/RoughDistribution.h
  math/Sampling.h
  math/Scattering.h
  math/ShadingVector.h
  math/SIMath.h
  math/SIMathConstants.h
  math/SIMathStd.h
  math/SIMathUtils.h
  math/SphereMap.h
  math/Spherical.h
  math/SplitSample.h
  math/Tangent.h
  math/Triangulation.h
  math/Triangulation2D.cpp
  math/Triangulation3D.cpp
  memory/AlignedAllocator.h
  memory/MemoryPool.h
  memory/MemoryPool.inl
  memory/MemoryStack.h
  memory/MemoryStack.inl
  serialization/BufferedNetworkSerializer.h
  serialization/BufferedNetworkSerializer.cpp
  serialization/BufferSerializer.h
  serialization/BufferSerializer.cpp
  serialization/FileSerializer.h
  serialization/FileSerializer.cpp
  serialization/ISerializable.h
  serialization/MemorySerializer.h
  serialization/MemorySerializer.cpp
  serialization/NetworkSerializer.h
  serialization/NetworkSerializer.cpp
  serialization/Serializer.h
  serialization/Serializer.cpp
  serialization/Serializer.inl
)

add_library(pr_lib_base ${PR_Src})
add_dependencies(pr_lib_base prhwinfo)
if(NOT WIN32)
  target_link_libraries(pr_lib_base PUBLIC Threads::Threads)
endif()
target_link_libraries(pr_lib_base PUBLIC Eigen3::Eigen PRIVATE ${CMAKE_DL_LIBS})
if(WIN32)
  target_link_libraries(pr_lib_base PRIVATE wsock32 ws2_32)
endif()
target_include_directories(pr_lib_base PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${INCLUDE_PREFIX}/>)
set_target_properties(pr_lib_base PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_compile_features(pr_lib_base PUBLIC cxx_std_17)
target_compile_definitions(pr_lib_base PUBLIC "$<$<CONFIG:Debug>:PR_DEBUG>")
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(pr_lib_base PUBLIC "PR_LIB_BASE_STATIC")
  set_target_properties(pr_lib_base PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
target_compile_definitions(pr_lib_base PRIVATE "PR_LIB_BASE_BUILD")
add_lto(pr_lib_base)
if(BUILD_SHARED_LIBS)
  strip_binary(pr_lib_base)
endif()

target_setup_deploy(pr_lib_base RECURSIVE)

if(BUILD_SHARED_LIBS)
  set(INSTALL_COMPONENT runtime)
else()
  set(INSTALL_COMPONENT development)
endif()
set_target_properties(pr_lib_base PROPERTIES EXPORT_NAME base)
install(TARGETS pr_lib_base COMPONENT ${INSTALL_COMPONENT} EXPORT PearRay-targets)
installHeaderFiles(${PR_Src})