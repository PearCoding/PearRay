MESSAGE(STATUS "Building PearRayDiagnostic")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(Src
  main.cpp
  3d/Camera.cpp
  3d/GraphicObject.cpp
  3d/OrbitCamera.cpp
  cnt/Container.cpp
  dialogs/EXRWindow.cpp
  dialogs/MainWindow.cpp
  dialogs/ProfWindow.cpp
  dialogs/SceneWindow.cpp
  io/EXRFile.cpp
  io/ImageBufferView.cpp
  io/ProfFile.cpp
  prof/ProfTreeItem.cpp
  prof/ProfTreeModel.cpp
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
  widgets/ProfPlotWidget.cpp
  widgets/PropertyView.cpp
  widgets/RangeSlider.cpp
  widgets/ToneMapperEditor.cpp
  widgets/View3DWidget.cpp)

set(Hdr
  3d/Camera.h
  3d/GraphicObject.h
  3d/OrbitCamera.h
  cnt/Container.h
  dialogs/EXRWindow.h
  dialogs/MainWindow.h
  dialogs/ProfWindow.h
  dialogs/SceneWindow.h
  io/EXRFile.h
  io/ImageBufferView.h
  io/ProfFile.h
  mapper/ToneMapper.h
  prof/ProfTreeItem.h
  prof/ProfTreeModel.h
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
  widgets/ProfPlotWidget.h
  widgets/PropertyView.h
  widgets/RangeSlider.h
  widgets/ToneMapperEditor.h
  widgets/View3DWidget.h)

set(Fms
  ui/EXRWindow.ui
  ui/MainWindow.ui
  ui/ProfWindow.ui
  ui/SceneWindow.ui
  ui/SpecWindow.ui
  ui/ToneMapperEditor.ui)

set(Res
  resources/icons.qrc)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})
QT5_ADD_RESOURCES(Res_Rcc ${Res})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr} ${Res_Rcc})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_executable(prdiag ${Src} ${Hdr} ${GenFiles})
target_link_libraries(prdiag PRIVATE Qt5::Widgets Qt5::Charts OpenImageIO::OpenImageIO TBB::tbb)
add_lto(prdiag)
strip_binary(prdiag)