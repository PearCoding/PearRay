CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay CNT Viewer")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS} ../utils)
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(PR_Main_Src
	main.cpp
  Container.cpp
  OrbitCamera.cpp)
SET(PR_Dialogs_Src
	dialogs/MainWindow.cpp)
SOURCE_GROUP("Source Files\\Dialogs" FILES ${PR_Dialogs_Src})
SET(PR_Widgets_Src
  widgets/View3DWidget.cpp)
SOURCE_GROUP("Source Files\\Widgets" FILES ${PR_Widgets_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Dialogs_Src} ${PR_Widgets_Src}
)

SET(PR_Main_Hdr
  Container.h
  OrbitCamera.h)
SET(PR_Dialogs_Hdr
  dialogs/MainWindow.h)
SOURCE_GROUP("Header Files\\Dialogs" FILES ${PR_Dialogs_Hdr})
SET(PR_Widgets_Hdr 
  widgets/View3DWidget.h)
SOURCE_GROUP("Header Files\\Widgets" FILES ${PR_Widgets_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Dialogs_Hdr} ${PR_Widgets_Hdr}
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

add_executable(prviewcnt ${PR_Src} ${PR_Hdr} ${PR_Fms} ${PR_Res}
	${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})
# target_link_libraries(prviewcnt pr_lib)
# target_link_libraries(prviewcnt pr_lib_utils)
target_link_libraries(prviewcnt Qt5::Widgets)
set_target_properties(prviewcnt PROPERTIES VERSION ${PR_Version})
target_compile_definitions(prviewcnt PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")