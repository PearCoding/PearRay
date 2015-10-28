CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  ${PR_Root}/FileLogListener.cpp
  ${PR_Root}/Logger.cpp
  ${PR_Root}/main.cpp)
SET(PR_Geometry_Src
  ${PR_Root}/geometry/Face.cpp
  ${PR_Root}/geometry/Mesh.cpp
  ${PR_Root}/geometry/Normal.cpp
  ${PR_Root}/geometry/UV.cpp
  ${PR_Root}/geometry/Vertex.cpp)
SOURCE_GROUP("Source Files\\Geometry" FILES ${PR_Geometry_Src})
SET(PR_Loader_Src
  ${PR_Root}/loader/WavefrontLoader.cpp)
SOURCE_GROUP("Source Files\\Loader" FILES ${PR_Loader_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Geometry_Src} ${PR_Loader_Src}
)

SET(PR_Main_Hdr
  ${PR_Root}/Config.h.in
  ${PR_Root}/FileLogListener.h
  ${PR_Root}/Logger.h
  ${PR_Root}/LogListener.h)
SET(PR_Geometry_Hdr
  ${PR_Root}/geometry/Face.h
  ${PR_Root}/geometry/Mesh.h
  ${PR_Root}/geometry/Normal.h
  ${PR_Root}/geometry/UV.h
  ${PR_Root}/geometry/Vertex.h)
SOURCE_GROUP("Header Files\\Geometry" FILES ${PR_Geometry_Hdr})
SET(PR_Loader_Hdr
  ${PR_Root}/loader/MeshLoader.h
  ${PR_Root}/loader/WavefrontLoader.h)
SOURCE_GROUP("Header Files\\Loader" FILES ${PR_Loader_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Geometry_Hdr} ${PR_Loader_Hdr}
)

SET(PR_TARGET pearray)

add_executable(${PR_TARGET} ${PR_Src} ${PR_Hdr})
target_link_libraries(${PR_TARGET} ${DL_TARGET})
target_link_libraries(${PR_TARGET} ${PP_TARGET})
set_target_properties(${PR_TARGET} PROPERTIES VERSION ${PR_Version})
set_target_properties(${PR_TARGET} PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")