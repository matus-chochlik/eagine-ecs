# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_library(
	eagine-ecs-elements-common
	EXCLUDE_FROM_ALL
	init.cpp
)
eagine_add_lib_analysis(eagine-ecs-elements-common)
eagine_embed_target_resources(
	TARGET eagine-ecs-elements-common
	PACKED
)
eagine_target_modules(
	eagine-ecs-elements-common
	std
	eagine.core
	eagine.ecs
)
set_target_properties(
	eagine-ecs-elements-common
	PROPERTIES FOLDER "Example/ECS"
)

macro(eagine_ecs_example EXAMPLE)
	add_executable(
		eagine-ecs-elements-${EXAMPLE}
		EXCLUDE_FROM_ALL
		main_${EXAMPLE}.cpp
	)
	eagine_add_exe_analysis(eagine-ecs-elements-${EXAMPLE})
	add_dependencies(eagine-ecs-examples eagine-ecs-elements-${EXAMPLE})
	eagine_target_modules(
		eagine-ecs-elements-${EXAMPLE}
		eagine.core
		eagine.ecs
	)
	target_link_libraries(
		eagine-ecs-elements-${EXAMPLE}
		PRIVATE eagine-ecs-elements-common
	)

	set_target_properties(
		eagine-ecs-elements-${EXAMPLE}
		PROPERTIES FOLDER "Example/ECS"
	)
	eagine_install_example(eagine-ecs-elements-${EXAMPLE})
endmacro()

eagine_ecs_example(basic)
eagine_ecs_example(decay_chain)

eagine_add_package_property(
	ecs-examples
	GENERATOR DEBIAN
	VARIABLE PACKAGE_DEPENDS
	VALUE "\${CXX_RUNTIME_PKGS},\${EAGINE_CORE_RUNTIME_PKGS}")

eagine_add_package_property(
	ecs-examples
	GENERATOR DEBIAN
	VARIABLE DESCRIPTION
	VALUE "EAGine ECS examples.")

