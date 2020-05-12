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
  config/Build.cpp
  config/Build.h
  config/ColliderOptions.h.in
  config/Constants.inl
  config/HW_Switch.inl.in
  config/SIMD.inl
  config/TriangleOptions.h.in
  config/Types.inl
  config/VectorGlue.inl
  config/Version.h.in
  curve/Curve.h
  log/ConsoleLogListener.cpp
  log/ConsoleLogListener.h
  log/FileLogListener.cpp
  log/FileLogListener.h
  log/LogListener.h
  math/Compression.h
  math/Fresnel.h
  math/Generator.h
  math/HemiMap.h
  math/ImportanceSampling.h
  math/Microfacet.h
  math/Projection.h
  math/ProjectionMap.h
  math/Reflection.h
  math/SIMath.h
  math/SIMathConstants.h
  math/SIMathStd.h
  math/SIMathUtils.h
  math/SphereMap.h
  math/Spherical.h
  math/Tangent.h
  memory/MemoryPool.h
  memory/MemoryPool.inl
  memory/MemoryStack.h
  memory/MemoryStack.inl
)

if(${CMAKE_VERSION} VERSION_LESS 3.15)
  set(_BOOST_HEADERS Boost::boost)
else()
  set(_BOOST_HEADERS Boost::headers)
endif()

add_library(pr_lib_base ${PR_Src})
add_dependencies(pr_lib_base prhwinfo)
if(NOT WIN32)
  target_link_libraries(pr_lib_base PUBLIC ${CMAKE_DL_LIBS} Threads::Threads)
endif()
target_link_libraries(pr_lib_base PUBLIC ${_BOOST_HEADERS} Boost::filesystem Eigen3::Eigen TBB::tbb)
target_include_directories(pr_lib_base PUBLIC PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
set_target_properties(pr_lib_base PROPERTIES CXX_VISIBILITY_PRESET hidden)
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