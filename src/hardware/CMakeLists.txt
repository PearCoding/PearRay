add_executable(prhwinfo hwinfo.cpp)
add_custom_command(
  TARGET prhwinfo
  POST_BUILD
  COMMAND "$<TARGET_FILE:prhwinfo>")

function(detect_hardware)

endfunction()