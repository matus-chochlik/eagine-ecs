# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_custom_target(eagine-ecs-examples ALL)
set_target_properties(
	eagine-ecs-examples
	PROPERTIES FOLDER "Example/ECS"
)

function(eagine_install_example TARGET_NAME)
	install(
		TARGETS ${TARGET_NAME}
		DESTINATION share/eagine/example/ecs
	)
endfunction()

function(eagine_example_common EXAMPLE_NAME)
	add_executable(
		eagine-${EXAMPLE_NAME}
		EXCLUDE_FROM_ALL
		"${EXAMPLE_NAME}.cpp"
		${ARGN}
	)
	add_dependencies(eagine-ecs-examples eagine-${EXAMPLE_NAME})
	if(${WITH_MODULES})
		eagine_target_modules(
			eagine-${EXAMPLE_NAME}
			eagine.core
			eagine.ecs
		)
	else()
		eagine_add_exe_analysis(eagine-${EXAMPLE_NAME})
		target_link_libraries(
			eagine-${EXAMPLE_NAME}
			PUBLIC EAGine::ECS
			PRIVATE ${ARGN}
		)
	endif()

	set_target_properties(
		eagine-${EXAMPLE_NAME}
		PROPERTIES FOLDER "Example/ECS"
	)
	eagine_install_example(eagine-${EXAMPLE_NAME})
endfunction()

eagine_example_common(001_hello)
eagine_example_common(002_space_opera)

if(NOT ${WITH_MODULES})
	add_subdirectory(elements)
endif()
