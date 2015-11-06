CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay Viewer")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

SET(PR_Viewer ${PR_Root}/viewer)
SET(PR_Main_Src
	${PR_Viewer}/main.cpp
)
SET(PR_Dialogs_Src
	${PR_Viewer}/dialogs/MainWindow.cpp
)
SOURCE_GROUP("Source Files\\Dialogs" FILES ${PR_Dialogs_Src})
SET(PR_Src ${PR_Main_Src} ${PR_Dialogs_Src}
)

SET(PR_Main_Hdr
)
SET(PR_Dialogs_Hdr
  ${PR_Viewer}/dialogs/MainWindow.h)
SOURCE_GROUP("Header Files\\Dialogs" FILES ${PR_Dialogs_Hdr})
SET(PR_Hdr ${PR_Main_Hdr} ${PR_Dialogs_Hdr}
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
target_link_libraries(rayview Qt5::Widgets)
target_link_libraries(rayview ${DL_TARGET})
target_link_libraries(rayview ${PP_TARGET})
set_target_properties(rayview PROPERTIES VERSION ${PR_Version})
set_target_properties(rayview PROPERTIES COMPILE_DEFINITIONS "${DEFS};" COMPILE_DEFINITIONS_DEBUG "${DEFS_DEBUG}")