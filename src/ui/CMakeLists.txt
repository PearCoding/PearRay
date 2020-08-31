MESSAGE(STATUS "Building UI library")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Src
  3d/Camera.cpp
  3d/GraphicObject.cpp
  3d/OrbitCamera.cpp
  io/EXRFile.cpp
  io/ImageBufferView.cpp
  properties/ColorProperty.cpp
  properties/DoubleProperty.cpp
  properties/GroupProperty.cpp
  properties/IntProperty.cpp
  properties/IProperty.cpp
  properties/PropertyTable.cpp
  properties/SelectionProperty.cpp
  properties/TextProperty.cpp
  widgets/ColorButton.cpp
  widgets/ImageView.cpp
  widgets/PropertyView.cpp
  widgets/RangeSlider.cpp
  widgets/ToneMapperEditor.cpp
  widgets/View3DWidget.cpp)

set(Hdr
  3d/Camera.h
  3d/GraphicObject.h
  3d/OrbitCamera.h
  io/EXRFile.h
  io/ImageBufferView.h
  mapper/ToneMapper.h
  properties/ColorProperty.h
  properties/DoubleProperty.h
  properties/GroupProperty.h
  properties/IntProperty.h
  properties/IProperty.h
  properties/PropertyTable.h
  properties/SelectionProperty.h
  properties/TextProperty.h
  widgets/ColorButton.h
  widgets/ImageView.h
  widgets/PropertyView.h
  widgets/RangeSlider.h
  widgets/ToneMapperEditor.h
  widgets/View3DWidget.h)

set(Fms
  ui/ToneMapperEditor.ui)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_library(pr_lib_ui ${Src} ${Hdr} ${GenFiles})
target_link_libraries(pr_lib_ui 
  PUBLIC pr_lib_base Qt5::Widgets 
  PRIVATE OpenImageIO::OpenImageIO TBB::tbb)
target_include_directories(pr_lib_ui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(pr_lib_ui PUBLIC "PR_LIB_UI_STATIC")
  set_target_properties(pr_lib_ui PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
target_compile_definitions(pr_lib_ui PRIVATE "PR_LIB_UI_BUILD")
add_lto(pr_lib_ui)
if(BUILD_SHARED_LIBS)
  strip_binary(pr_lib_ui)
endif()