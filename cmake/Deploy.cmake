function (_target_deploy_add_dependency TARGET ALLOW_REC)
	foreach(dependency ${ARGN})
		get_target_property(type ${dependency} TYPE)
		
		if(NOT ${type} EQUAL "INTERFACE_LIBRARY")
			get_property(has_loc TARGET ${dependency} PROPERTY IMPORTED_LOCATION SET)
			if(${has_loc})
				get_target_property(loc ${dependency} IMPORTED_LOCATION)
				add_custom_command(TARGET ${TARGET} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different "${loc}" "$<TARGET_FILE_DIR:${TARGET}>"
					COMMENT "Copying ${loc} to $<TARGET_FILE_DIR:${TARGET}>")
				install(FILES "${loc}" TYPE BIN COMPONENT runtime)
			endif()
		endif()
		
		if(${ALLOW_REC})
			get_target_property(libs ${dependency} INTERFACE_LINK_LIBRARIES)
			foreach(lib ${libs})
				if(TARGET ${lib})
					_target_deploy_add_dependency(${TARGET} ${ALLOW_REC} ${lib})
				endif()
			endforeach()
		endif()
	endforeach()
endfunction()

function (target_setup_deploy TARGET)
set(options QT RECURSIVE)
set(multiValueArgs LIBRARIES)
cmake_parse_arguments(ARGS "${options}" "" "${multiValueArgs}" "${ARGN}")

if(NOT WIN32)
	return()
endif()

if(${ARGS_QT})
	get_target_property(qt_loc Qt5::Widgets INTERFACE_LINK_DIRECTORIES)
	find_program(QT_WINDEPLOY windeployqt
		PATHS ${qt_loc})
	if(NOT QT_WINDEPLOY)
		message(WARNING "Could not deploy Qt as windeployqt was not found!")
	else()
		add_custom_command(TARGET ${TARGET} POST_BUILD
			COMMAND ${QT_WINDEPLOY} 
				$<IF:$<CONFIG:Debug>,--debug,--release> 
				--verbose 0
				--no-compiler-runtime
				"$<TARGET_FILE:${TARGET}>"
			COMMENT "Copying Qt files to $<TARGET_FILE_DIR:${TARGET}>")
		
		file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_path" CONTENT "$<TARGET_FILE:${TARGET}>")

		install(CODE
			"
			file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_path\" _file)
			execute_process(
				COMMAND \"${CMAKE_COMMAND}\" -E
					env PATH=\"${qt_loc}\" \"${QT_WINDEPLOY}\"
						--dry-run
					    --no-compiler-runtime
						--no-angle
						--no-opengl-sw
						--list mapping
						\${_file}
				OUTPUT_VARIABLE _output
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			separate_arguments(_files WINDOWS_COMMAND \${_output})
			while(_files)
				list(GET _files 0 _src)
				list(GET _files 1 _dest)
				execute_process(
					COMMAND \"${CMAKE_COMMAND}\" -E
						copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/\${_dest}\"
				)
				list(REMOVE_AT _files 0 1)
			endwhile()
			"
		)
	endif()
endif()

_target_deploy_add_dependency(${TARGET} ${ARGS_RECURSIVE} ${ARGS_LIBRARIES})
endfunction()