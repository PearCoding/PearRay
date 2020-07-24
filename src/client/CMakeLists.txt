set(Src
  EnumOption.h
  ImageUpdateObserver.cpp
  ImageUpdateObserver.h
  IProgressObserver.h
  main.cpp
  ProgramSettings.cpp
  ProgramSettings.h
  StatusObserver.cpp
  StatusObserver.h
  Utils.h
)

add_executable(pearray ${Src})
target_link_libraries(pearray PRIVATE pr_lib_loader)
add_lto(pearray)
strip_binary(pearray)
