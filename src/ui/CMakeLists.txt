MESSAGE(STATUS "Building UI library")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Src
  3d/Camera.cpp
  3d/glad.c
  3d/entities/AABBEntity.cpp
  3d/entities/Axis3DEntity.cpp
  3d/entities/BoxEntity.cpp
  3d/entities/ConeEntity.cpp
  3d/entities/CylinderEntity.cpp
  3d/entities/DiskEntity.cpp
  3d/entities/GraphicEntity.cpp
  3d/entities/GridEntity.cpp
  3d/entities/GridMapEntity.cpp
  3d/entities/HemiFunctionEntity.cpp
  3d/entities/InstanceEntity.cpp
  3d/entities/MeshEntity.cpp
  3d/entities/PointMapEntity.cpp
  3d/entities/ScreenSpaceEntity.cpp
  3d/entities/SphereEntity.cpp
  3d/entities/TransformableEntity.cpp
  3d/shader/BackgroundShader.cpp
  3d/shader/ColorShader.cpp
  3d/shader/GoochShader.cpp
  3d/shader/Shader.cpp
  3d/shader/ShaderProgram.cpp
  3d/shader/WireframeShader.cpp
  io/ImageBufferView.cpp
  io/ImageFile.cpp
  io/ImageLayer.cpp
  log/LogTableModel.cpp
  log/UILogListener.cpp
  properties/BoolProperty.cpp
  properties/ColorProperty.cpp
  properties/DoubleProperty.cpp
  properties/GroupProperty.cpp
  properties/IntProperty.cpp
  properties/IProperty.cpp
  properties/PropertyContainer.cpp
  properties/PropertyItemDelegate.cpp
  properties/PropertyTreeModel.cpp
  properties/SelectionProperty.cpp
  properties/TextProperty.cpp
  properties/Vector3Property.cpp
  widgets/ColorButton.cpp
  widgets/ImagePipelineEditor.cpp
  widgets/ImageView.cpp
  widgets/PropertyView.cpp
  widgets/RangeSlider.cpp
  widgets/View3DWidget.cpp)

set(Hdr
  3d/Camera.h
  3d/glad.h
  3d/khrplatform.h
  3d/OpenGLHeaders.h
  3d/ShadingContext.h
  3d/entities/AABBEntity.h
  3d/entities/Axis3DEntity.h
  3d/entities/BoxEntity.h
  3d/entities/ConeEntity.h
  3d/entities/CylinderEntity.h
  3d/entities/DiskEntity.h
  3d/entities/GraphicEntity.h
  3d/entities/GridEntity.h
  3d/entities/GridMapEntity.h
  3d/entities/HemiFunctionEntity.h
  3d/entities/InstanceEntity.h
  3d/entities/MeshEntity.h
  3d/entities/PointMapEntity.h
  3d/entities/ScreenSpaceEntity.h
  3d/entities/SphereEntity.h
  3d/entities/TransformableEntity.h
  3d/shader/BackgroundShader.h
  3d/shader/ColorShader.h
  3d/shader/GoochShader.h
  3d/shader/Shader.h
  3d/shader/ShaderProgram.h
  3d/shader/WireframeShader.h
  io/ImageBufferIOView.h
  io/ImageBufferView.h
  io/ImageFile.h
  io/ImageLayer.h
  log/LogEntry.h
  log/LogTableModel.h
  log/UILogListener.h
  mapper/ImagePipeline.h
  properties/BoolProperty.h
  properties/ColorProperty.h
  properties/DoubleProperty.h
  properties/GroupProperty.h
  properties/IntProperty.h
  properties/IProperty.h
  properties/PropertyContainer.h
  properties/PropertyItemDelegate.h
  properties/PropertyTreeModel.h
  properties/SelectionProperty.h
  properties/TextProperty.h
  properties/Vector3Property.h
  widgets/ColorButton.h
  widgets/ImagePipelineEditor.h
  widgets/ImageView.h
  widgets/PropertyView.h
  widgets/RangeSlider.h
  widgets/View3DWidget.h)

set(Fms
  ui/ImagePipelineEditor.ui)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_library(pr_lib_ui ${Src} ${Hdr} ${GenFiles})
target_link_libraries(pr_lib_ui 
  PUBLIC pr_lib_loader Qt5::Widgets 
  PRIVATE OpenImageIO::OpenImageIO TBB::tbb)
if(WIN32)
  target_link_libraries(pr_lib_ui PRIVATE OpenEXR::IlmImf)
endif()
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

# Better do not include this in the actual release!
if(BUILD_SHARED_LIBS)
  install(TARGETS pr_lib_ui COMPONENT viewer)
endif()