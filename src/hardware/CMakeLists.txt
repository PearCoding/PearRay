add_executable(prhwinfo hwinfo.cpp)

set(out_file "${CMAKE_BINARY_DIR}/PR_HW_Config.inl")
add_custom_command(TARGET prhwinfo
  POST_BUILD
  COMMAND "$<TARGET_FILE:prhwinfo>" "--header" "${out_file}"
  BYPRODUCTS "${out_file}"
  COMMENT "Generating hardware feature information: ${out_file}" 
  VERBATIM)