MESSAGE(STATUS "Building prviewer")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Src
  main.cpp
  FrameBufferView.cpp
  Project.cpp
  dialogs/MainWindow.cpp
  widgets/WinExtraWidget.cpp
  )

set(Hdr
  FrameBufferView.h
  Project.h
  dialogs/MainWindow.h
  widgets/WinExtraWidget.h
  )

set(Fms
  ui/MainWindow.ui
  )

set(Res
  ../../ui/resources/icons.qrc)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})
QT5_ADD_RESOURCES(Res_Rcc ${Res})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr} ${Res_Rcc})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_executable(prviewer ${Src} ${Hdr} ${GenFiles})
target_link_libraries(prviewer PRIVATE pr_lib_ui)
if(TARGET Qt5::WinExtras)
	target_link_libraries(prviewer PRIVATE Qt5::WinExtras)
	target_compile_definitions(prviewer PRIVATE PR_UI_HAS_WINEXTRAS)
endif()
add_lto(prviewer)
strip_binary(prviewer)
target_setup_deploy(prviewer QT RECURSIVE)

install(TARGETS prviewer COMPONENT viewer)