# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
eagine_add_module(
	eagine.ecs PARTITION entity_traits
)

eagine_add_module(
	eagine.ecs PARTITION component
	IMPORTS
		eagine.core.types
		eagine.core.container
)

eagine_add_module(
	eagine.ecs PARTITION manipulator
	IMPORTS
		eagine.core.types
)

eagine_add_module(
	eagine.ecs PARTITION storage
	IMPORTS
		eagine.core.types
		eagine.core.reflection
)

eagine_add_module(
	eagine.ecs PARTITION cmp_storage
	IMPORTS
		entity_traits manipulator storage
		eagine.core.types
		eagine.core.utility
)

eagine_add_module(
	eagine.ecs PARTITION rel_storage
	IMPORTS
		entity_traits manipulator storage
		eagine.core.types
		eagine.core.utility
)

eagine_add_module(
	eagine.ecs PARTITION std_map_storage
	IMPORTS
		entity_traits manipulator component
		storage cmp_storage rel_storage
		eagine.core.types
		eagine.core.string
		eagine.core.utility
)

eagine_add_module(
	eagine.ecs
	PP_NAME ECS
	INTERFACES ecs
	PARTITIONS
		entity_traits
		component
		manipulator
		storage
		cmp_storage
		rel_storage
		std_map_storage
	SOURCES
)

