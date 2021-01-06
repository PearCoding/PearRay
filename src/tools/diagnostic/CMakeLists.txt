MESSAGE(STATUS "Building prdiag")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Src
  main.cpp
  cnt/Container.cpp
  dialogs/ImageWindow.cpp
  dialogs/MainWindow.cpp
  dialogs/ProfWindow.cpp
  io/ProfFile.cpp
  prof/ProfTreeItem.cpp
  prof/ProfTreeModel.cpp
  rdmp/RayArray.cpp
  widgets/ProfPlotWidget.cpp)

set(Hdr
  cnt/Container.h
  dialogs/ImageWindow.h
  dialogs/MainWindow.h
  dialogs/ProfWindow.h
  io/ProfFile.h
  prof/ProfTreeItem.h
  prof/ProfTreeModel.h
  rdmp/RayArray.h
  widgets/ProfPlotWidget.h)

set(Fms
  ui/ImageWindow.ui
  ui/MainWindow.ui
  ui/ProfWindow.ui
  ui/SpecWindow.ui)

set(Res
  ../../ui/resources/icons.qrc)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})
QT5_ADD_RESOURCES(Res_Rcc ${Res})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr} ${Res_Rcc})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_executable(prdiag ${Src} ${Hdr} ${GenFiles})
target_link_libraries(prdiag PRIVATE pr_lib_ui Qt5::Charts)
add_lto(prdiag)
strip_binary(prdiag)
target_setup_deploy(prdiag QT RECURSIVE)

install(TARGETS prdiag COMPONENT viewer)