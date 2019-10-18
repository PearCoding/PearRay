CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRayDiagnostic")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(Src
  main.cpp
  3d/Camera.cpp
  3d/GraphicObject.cpp
  3d/OrbitCamera.cpp
  cnt/Container.cpp
  dialogs/EXRWindow.cpp
  dialogs/MainWindow.cpp
  dialogs/SceneWindow.cpp
  dialogs/SpecWindow.cpp
  io/EXRFile.cpp
  io/ImageBufferView.cpp
  io/SpecFile.cpp
  properties/ColorProperty.cpp
  properties/DoubleProperty.cpp
  properties/GroupProperty.cpp
  properties/IntProperty.cpp
  properties/IProperty.cpp
  properties/PropertyTable.cpp
  properties/SelectionProperty.cpp
  properties/TextProperty.cpp
  rdmp/RayArray.cpp
  widgets/ColorButton.cpp
  widgets/ImageView.cpp
  widgets/PropertyView.cpp
  widgets/RangeSlider.cpp
  widgets/ToneMapperEditor.cpp
  widgets/View3DWidget.cpp)

SET(Hdr
  3d/Camera.h
  3d/GraphicObject.h
  3d/OrbitCamera.h
  cnt/Container.h
  dialogs/EXRWindow.h
  dialogs/MainWindow.h
  dialogs/SceneWindow.h
  dialogs/SpecWindow.h
  io/EXRFile.h
  io/ImageBufferView.h
  io/SpecFile.h
  mapper/ToneMapper.h
  properties/ColorProperty.h
  properties/DoubleProperty.h
  properties/GroupProperty.h
  properties/IntProperty.h
  properties/IProperty.h
  properties/PropertyTable.h
  properties/SelectionProperty.h
  properties/TextProperty.h
  rdmp/RayArray.h
  widgets/ColorButton.h
  widgets/ImageView.h
  widgets/PropertyView.h
  widgets/RangeSlider.h
  widgets/ToneMapperEditor.h
  widgets/View3DWidget.h)

SET(Fms
  ui/EXRWindow.ui
  ui/MainWindow.ui
  ui/SceneWindow.ui
  ui/SpecWindow.ui
  ui/ToneMapperEditor.ui)

SET(Res
  resources/icons.qrc)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})
QT5_ADD_RESOURCES(Res_Rcc ${Res})

SET(GenFiles ${Hdr_Moc} ${Fms_Hdr} ${Res_Rcc})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

# disable autolinking to qtmain
cmake_policy(SET CMP0020 OLD)

add_executable(prdiag ${Src} ${Hdr} ${GenFiles})
target_link_libraries(prdiag Qt5::Widgets ${OPENEXR_LIBRARIES} ${Boost_LIBRARIES})
set_target_properties(prdiag PROPERTIES VERSION ${PR_Version})
target_compile_definitions(prdiag PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")