CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay Viewer")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS} ../utils)
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(PR_Main_Src
	main.cpp
)
SET(PR_Dialogs_Src
	dialogs/MainWindow.cpp
)
SOURCE_GROUP("Source Files\\Dialogs" FILES ${PR_Dialogs_Src})
SET(PR_Models_Src
  models/EntityTreeModel.cpp)
SOURCE_GROUP("Source Files\\Models" FILES ${PR_Models_Src})
SET(PR_Properties_Src
  properties/BoolProperty.cpp
  properties/ButtonProperty.cpp
  properties/ColorProperty.cpp
  properties/DoubleProperty.cpp
  properties/GroupProperty.cpp
  properties/IntProperty.cpp
  properties/IProperty.cpp
  properties/PropertyTable.cpp
  properties/SelectionProperty.cpp
  properties/TextProperty.cpp
  properties/VectorProperty.cpp)
SOURCE_GROUP("Source Files\\Properties" FILES ${PR_Properties_Src})
SET(PR_Widgets_Src
  widgets/ColorButton.cpp
  widgets/EntityDetailsView.cpp
  widgets/PropertyView.cpp
  widgets/SpectrumWidget.cpp
  widgets/SystemPropertyView.cpp
  widgets/ViewWidget.cpp)
SOURCE_GROUP("Source Files\\Widgets" FILES ${PR_Widgets_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Dialogs_Src} ${PR_Widgets_Src}
  ${PR_Properties_Src} ${PR_Models_Src}
)

SET(PR_Main_Hdr
)
SET(PR_Dialogs_Hdr
  dialogs/MainWindow.h)
SOURCE_GROUP("Header Files\\Dialogs" FILES ${PR_Dialogs_Hdr})
SET(PR_Models_Hdr
  models/EntityTreeModel.h)
SOURCE_GROUP("Header Files\\Models" FILES ${PR_Models_Hdr})
SET(PR_Properties_Hdr
  properties/BoolProperty.h
  properties/ButtonProperty.h
  properties/ColorProperty.h
  properties/DoubleProperty.h
  properties/GroupProperty.h
  properties/IntProperty.h
  properties/IProperty.h
  properties/PropertyTable.h
  properties/SelectionProperty.h
  properties/TextProperty.h
  properties/VectorProperty.h)
SOURCE_GROUP("Header Files\\Properties" FILES ${PR_Properties_Hdr})
SET(PR_Widgets_Hdr
  widgets/ColorButton.h
  widgets/EntityDetailsView.h
  widgets/PropertyView.h
  widgets/SpectrumWidget.h
  widgets/SystemPropertyView.h
  widgets/ViewWidget.h)
SOURCE_GROUP("Header Files\\Widgets" FILES ${PR_Widgets_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Dialogs_Hdr} ${PR_Widgets_Hdr}
  ${PR_Properties_Hdr} ${PR_Models_Hdr}
)

SET(PR_Fms
  ui/MainWindow.ui)
SOURCE_GROUP("Form Files" FILES ${PR_Fms})

SET(PR_Res
  resources/icons.qrc)
SOURCE_GROUP("Resource Files" FILES ${PR_Res})

QT5_WRAP_CPP(PR_Hdr_Moc ${PR_Hdr})
QT5_WRAP_UI(PR_Fms_Hdr ${PR_Fms})
QT5_ADD_RESOURCES(PR_Res_Rcc ${PR_Res})

SOURCE_GROUP("Generated Files" FILES ${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})

# disable autolinking to qtmain
cmake_policy(SET CMP0020 OLD)

add_executable(rayview ${PR_Src} ${PR_Hdr} ${PR_Fms} ${PR_Res}
	${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})
target_link_libraries(rayview pr_lib)
target_link_libraries(rayview pr_lib_utils)
target_link_libraries(rayview Qt5::Widgets)
set_target_properties(rayview PROPERTIES VERSION ${PR_Version})
target_compile_definitions(rayview PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")