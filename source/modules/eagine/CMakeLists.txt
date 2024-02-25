# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.ecs PARTITION entity_traits
	IMPORTS
		std
		eagine.core.identifier
		eagine.core.reflection
)

eagine_add_module(
	eagine.ecs PARTITION component
	IMPORTS
		std entity_traits
		eagine.core.types
		eagine.core.identifier
		eagine.core.container
)

eagine_add_module(
	eagine.ecs PARTITION manipulator
	IMPORTS
		std
		eagine.core.types
)

eagine_add_module(
	eagine.ecs PARTITION storage
	IMPORTS
		std entity_traits
		manipulator
		eagine.core.types
		eagine.core.utility
		eagine.core.reflection
)

eagine_add_module(
	eagine.ecs PARTITION map_storage
	IMPORTS
		std entity_traits
		manipulator component
		storage
		eagine.core.types
		eagine.core.utility
		eagine.core.container
		eagine.core.reflection
)

eagine_add_module(
	eagine.ecs PARTITION manager
	IMPORTS
		std entity_traits
		manipulator component
		storage
		eagine.core.debug
		eagine.core.types
		eagine.core.string
		eagine.core.utility
		eagine.core.valid_if
)

eagine_add_module(
	eagine.ecs PARTITION object
	IMPORTS
		std component manipulator manager
		eagine.core.types
		eagine.core.identifier
		eagine.core.main_ctx
)

eagine_add_module(
	eagine.ecs
)

eagine_add_module_tests(
	eagine.ecs
	UNITS
		storage
		manager_std_map
		manager_flat_map
	IMPORTS
		std
		eagine.core
)
set_tests_properties(build-test.eagine.ecs.manager_std_map  PROPERTIES COST 10)
set_tests_properties(build-test.eagine.ecs.manager_flat_map PROPERTIES COST 10)
