CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PR_Main_Src
  NetworkDisplayDriver.cpp)
SET(PR_Src ${PR_Main_Src}
)

SET(PR_Main_Hdr
  NetworkDisplayDriver.h)
SET(PR_Hdr ${PR_Main_Hdr}
)

add_library(pr_lib_network SHARED ${PR_Src} ${PR_Hdr})
target_link_libraries(pr_lib_network pr_lib)
target_link_libraries(pr_lib_network ${DL_TARGET})
target_link_libraries(pr_lib_network ${Boost_LIBRARIES})

set_target_properties(pr_lib_network PROPERTIES VERSION ${PR_Version})
target_compile_definitions(pr_lib_network PRIVATE "${DEFS};PR_LIB_NETWORK_BUILD" "$<$<CONFIG:Debug>:${DEFS_DEBUG}>")
