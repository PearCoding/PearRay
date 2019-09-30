CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

MESSAGE(STATUS "Building PearRay CNT Viewer")
MESSAGE(STATUS "Using Qt: ${Qt5Widgets_VERSION_STRING}")

include_directories(${Qt5Widgets_INCLUDE_DIRS} ../utils)
add_definitions(${Qt5Widgets_DEFINITIONS})

IF(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

SET(PR_Src
	main.cpp
  Container.cpp
	dialogs/MainWindow.cpp)

SET(PR_Hdr
  Container.h
  dialogs/MainWindow.h)

SET(PR_Fms
  ui/MainWindow.ui)

SET(PR_Res
  ../utils/resources/icons.qrc)

QT5_WRAP_CPP(PR_Hdr_Moc ${PR_Hdr})
QT5_WRAP_UI(PR_Fms_Hdr ${PR_Fms})
QT5_ADD_RESOURCES(PR_Res_Rcc ${PR_Res})

SOURCE_GROUP("Generated Files" FILES ${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})

# disable autolinking to qtmain
cmake_policy(SET CMP0020 OLD)

add_executable(prviewcnt ${PR_Src} ${PR_Hdr} ${PR_Fms} ${PR_Res}
  ${PR_Hdr_Moc} ${PR_Fms_Hdr} ${PR_Res_Rcc})
target_link_libraries(prviewcnt pr_lib_viewutils)
target_link_libraries(prviewcnt Qt5::Widgets)
set_target_properties(prviewcnt PROPERTIES VERSION ${PR_Version})
target_compile_definitions(prviewcnt PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")