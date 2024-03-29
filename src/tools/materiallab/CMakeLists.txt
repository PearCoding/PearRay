MESSAGE(STATUS "Building prmateriallab")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(Src
  main.cpp
  MaterialCache.cpp
  dialogs/MainWindow.cpp
  dialogs/MaterialWindow.cpp)

set(Hdr
  MaterialCache.h
  dialogs/MainWindow.h
  dialogs/MaterialWindow.h)

set(Fms
  ui/MainWindow.ui
  ui/MaterialWindow.ui)

set(Res
  ../../ui/resources/icons.qrc)

QT5_WRAP_CPP(Hdr_Moc ${Hdr})
QT5_WRAP_UI(Fms_Hdr ${Fms})
QT5_ADD_RESOURCES(Res_Rcc ${Res})

set(GenFiles ${Hdr_Moc} ${Fms_Hdr} ${Res_Rcc})
SOURCE_GROUP("Generated Files" FILES ${GenFiles})

add_executable(prmateriallab ${Src} ${Hdr} ${GenFiles})
target_link_libraries(prmateriallab PRIVATE pr_lib_ui)
add_lto(prmateriallab)
strip_binary(prmateriallab)

install(TARGETS prmateriallab COMPONENT viewer)
