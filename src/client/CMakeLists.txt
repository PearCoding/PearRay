CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  main.cpp
  ProgramSettings.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  EnumOption.h
  ProgramSettings.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

include_directories(../utils)
IF(ASIO_FOUND)
  include_directories(../network)
ENDIF()

add_executable(pearray ${PR_Src} ${PR_Hdr})
target_link_libraries(pearray pr_lib)
target_link_libraries(pearray pr_lib_utils)

IF(ASIO_FOUND)
	target_link_libraries(pearray pr_lib_network)
ENDIF()

set_target_properties(pearray PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pearray PRIVATE "${DEFS}" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
