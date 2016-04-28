CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay Viewer")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(PR_Viewer ${PR_Root}/viewer)
SET(PR_Main_Src
	${PR_Viewer}/main.cpp
)
SET(PR_Dialogs_Src
	${PR_Viewer}/dialogs/MainWindow.cpp
)
SOURCE_GROUP("Source Files\\Dialogs" FILES ${PR_Dialogs_Src})
SET(PR_Models_Src
  ${PR_Viewer}/models/EntityTreeModel.cpp)
SOURCE_GROUP("Source Files\\Models" FILES ${PR_Models_Src})
SET(PR_Properties_Src
  ${PR_Viewer}/properties/BoolProperty.cpp
  ${PR_Viewer}/properties/ButtonProperty.cpp
  ${PR_Viewer}/properties/ColorProperty.cpp
  ${PR_Viewer}/properties/DoubleProperty.cpp
  ${PR_Viewer}/properties/GroupProperty.cpp
  ${PR_Viewer}/properties/IntProperty.cpp
  ${PR_Viewer}/properties/IProperty.cpp
  ${PR_Viewer}/properties/PropertyTable.cpp
  ${PR_Viewer}/properties/SelectionProperty.cpp
  ${PR_Viewer}/properties/TextProperty.cpp
  ${PR_Viewer}/properties/VectorProperty.cpp)
SOURCE_GROUP("Source Files\\Properties" FILES ${PR_Properties_Src})
SET(PR_Widgets_Src
  ${PR_Viewer}/widgets/ColorButton.cpp
  ${PR_Viewer}/widgets/EntityDetailsView.cpp
  ${PR_Viewer}/widgets/PropertyView.cpp
  ${PR_Viewer}/widgets/SpectrumWidget.cpp
  ${PR_Viewer}/widgets/SystemPropertyView.cpp
  ${PR_Viewer}/widgets/ViewWidget.cpp)
SOURCE_GROUP("Source Files\\Widgets" FILES ${PR_Widgets_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Dialogs_Src} ${PR_Widgets_Src}
  ${PR_Properties_Src} ${PR_Models_Src}
)

SET(PR_Main_Hdr
)
SET(PR_Dialogs_Hdr
  ${PR_Viewer}/dialogs/MainWindow.h)
SOURCE_GROUP("Header Files\\Dialogs" FILES ${PR_Dialogs_Hdr})
SET(PR_Models_Hdr
  ${PR_Viewer}/models/EntityTreeModel.h)
SOURCE_GROUP("Header Files\\Models" FILES ${PR_Models_Hdr})
SET(PR_Properties_Hdr
  ${PR_Viewer}/properties/BoolProperty.h
  ${PR_Viewer}/properties/ButtonProperty.h
  ${PR_Viewer}/properties/ColorProperty.h
  ${PR_Viewer}/properties/DoubleProperty.h
  ${PR_Viewer}/properties/GroupProperty.h
  ${PR_Viewer}/properties/IntProperty.h
  ${PR_Viewer}/properties/IProperty.h
  ${PR_Viewer}/properties/PropertyTable.h
  ${PR_Viewer}/properties/SelectionProperty.h
  ${PR_Viewer}/properties/TextProperty.h
  ${PR_Viewer}/properties/VectorProperty.h)
SOURCE_GROUP("Header Files\\Properties" FILES ${PR_Properties_Hdr})
SET(PR_Widgets_Hdr
  ${PR_Viewer}/widgets/ColorButton.h
  ${PR_Viewer}/widgets/EntityDetailsView.h
  ${PR_Viewer}/widgets/PropertyView.h
  ${PR_Viewer}/widgets/SpectrumWidget.h
  ${PR_Viewer}/widgets/SystemPropertyView.h
  ${PR_Viewer}/widgets/ViewWidget.h)
SOURCE_GROUP("Header Files\\Widgets" FILES ${PR_Widgets_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Dialogs_Hdr} ${PR_Widgets_Hdr}
  ${PR_Properties_Hdr} ${PR_Models_Hdr}
)

SET(PR_Fms
  ${PR_Viewer}/ui/MainWindow.ui)
SOURCE_GROUP("Form Files" FILES ${PR_Fms})

QT5_WRAP_CPP(PR_Hdr_Moc ${PR_Hdr})
QT5_WRAP_UI(PR_Fms_Hdr ${PR_Fms})
#QT5_ADD_RESOURCES(PR_Res_Rcc ${PR_Res})

SOURCE_GROUP("Generated Files" FILES ${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})

add_executable(rayview ${PR_Src} ${PR_Hdr} ${PR_Fms}
	${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})
target_link_libraries(rayview ${PR_TARGET})
target_link_libraries(rayview pr_lib_utils)
target_link_libraries(rayview Qt5::Widgets)
target_link_libraries(rayview ${PP_TARGET})
set_target_properties(rayview PROPERTIES VERSION ${PR_Version})
set_target_properties(rayview PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")