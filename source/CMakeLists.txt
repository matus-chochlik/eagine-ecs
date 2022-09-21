# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
add_subdirectory(modules)

set(EAGINE_ECS_EXPORT_TARGETS)

if(TARGET eagine-ecs)
	add_library(EAGine::ECS INTERFACE IMPORTED GLOBAL)
	target_link_libraries(
		EAGine::ECS
		INTERFACE
			eagine-ecs-headers
			eagine-ecs
	)
	list(
		APPEND EAGINE_ECS_EXPORT_TARGETS
			eagine-ecs-private-headers
			eagine-ecs-headers
			eagine-ecs
	)

	install(
		EXPORT ECS
		NAMESPACE EAGine::
		FILE ECS.cmake
		DESTINATION lib/cmake/EAGine
	)
endif()

install(
	TARGETS ${EAGINE_ECS_EXPORT_TARGETS}
	EXPORT ECS
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
	PUBLIC_HEADER DESTINATION include
)

