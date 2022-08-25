# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
add_library(
	eagine-ecs-elements-common
	EXCLUDE_FROM_ALL
	init.cpp
)
eagine_add_lib_analysis(eagine-ecs-elements-common)
eagine_embed_target_resources(TARGET eagine-ecs-elements-common PACKED)
if(${WITH_MODULES})
	eagine_target_modules(
		eagine-ecs-elements-common
		eagine.core
		eagine.ecs
	)
else()
	target_link_libraries(
		eagine-ecs-elements-common
		PUBLIC EAGine::ECS
	)
endif()
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
	if(${WITH_MODULES})
		eagine_target_modules(
			eagine-ecs-elements-${EXAMPLE}
			eagine.core
			eagine.ecs
		)
	endif()
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

