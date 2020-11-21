set(Src
  EnumOption.h
  ImageUpdateObserver.cpp
  ImageUpdateObserver.h
  IProgressObserver.h
  main.cpp
  NetworkObserver.cpp
  NetworkObserver.h
  ProgramSettings.cpp
  ProgramSettings.h
  StatusObserver.cpp
  StatusObserver.h
  TevObserver.cpp
  TevObserver.h
  Utils.h
)

add_executable(pearray ${Src})
target_link_libraries(pearray PRIVATE pr_lib_loader)
add_lto(pearray)
strip_binary(pearray)

install(TARGETS pearray COMPONENT runtime)