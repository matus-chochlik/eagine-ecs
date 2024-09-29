# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_custom_target(eagine-ecs-examples ALL)
set_target_properties(
	eagine-ecs-examples
	PROPERTIES FOLDER "Example/ECS"
)

function(eagine_install_example TARGET_NAME)
	install(
		TARGETS ${TARGET_NAME}
		COMPONENT ecs-examples
		DESTINATION share/eagine/example/ecs)
endfunction()

function(eagine_example_common EXAMPLE_NAME)
	add_executable(
		eagine-${EXAMPLE_NAME}
		EXCLUDE_FROM_ALL
		"${EXAMPLE_NAME}.cpp"
		${ARGN}
	)
	add_dependencies(eagine-ecs-examples eagine-${EXAMPLE_NAME})
	eagine_add_exe_analysis(eagine-${EXAMPLE_NAME})
	eagine_target_modules(
		eagine-${EXAMPLE_NAME}
		std
		eagine.core
		eagine.ecs
	)

	set_target_properties(
		eagine-${EXAMPLE_NAME}
		PROPERTIES FOLDER "Example/ECS"
	)
	eagine_install_example(eagine-${EXAMPLE_NAME})
endfunction()

eagine_example_common(001_hello_manager)
eagine_example_common(001_hello_object)
eagine_example_common(002_signals)
eagine_example_common(003_space_opera)
eagine_example_common(004_space_objects)

add_subdirectory(elements)

eagine_add_license(ecs-examples)
eagine_add_debian_changelog(ecs-examples)


