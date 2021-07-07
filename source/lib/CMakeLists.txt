# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt

add_library(
	eagine-ecs
	basic_manager.cpp
)
eagine_add_lib_analysis(eagine-ecs)

target_compile_definitions(
	eagine-ecs PUBLIC EAGINE_ECS_LIBRARY=1
)

target_include_directories(
	eagine-ecs
	PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_link_libraries(
	eagine-ecs
	PUBLIC
		EAGine::Core
		eagine-ecs-headers
	PRIVATE
		eagine-ecs-private-headers
)
