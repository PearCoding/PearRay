function(strip_binary TARGET)
    if (NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${TARGET}>)
    endif()
endfunction()