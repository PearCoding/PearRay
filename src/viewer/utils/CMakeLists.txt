CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay Viewer Utility Library")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(Src
  Camera.cpp
  GraphicObject.cpp
  OrbitCamera.cpp
  widgets/View3DWidget.cpp)

SET(Hdr
  Camera.h
  GraphicObject.h
  OrbitCamera.h
  widgets/View3DWidget.h)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})

SOURCE_GROUP("Generated Files" FILES ${Hdr_Moc})

# disable autolinking to qtmain
cmake_policy(SET CMP0020 OLD)

add_library(pr_lib_viewutils STATIC ${Src} ${Hdr} ${Hdr_Moc})
target_link_libraries(pr_lib_viewutils Qt5::Widgets)
set_target_properties(pr_lib_viewutils PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib_viewutils PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")