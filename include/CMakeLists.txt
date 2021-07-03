# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
#
include(./Sources.cmake)
# - regular headers ------------------------------------------------------------
add_library(eagine-ecs-headers INTERFACE)

target_include_directories(
	eagine-ecs-headers
	INTERFACE
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

foreach(FILENAME ${HEADERS} ${PUB_INLS})
	set_property(
		TARGET eagine-ecs-headers
		APPEND PROPERTY PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
	)
endforeach()
# - private headers or implementation files ------------------------------------
add_library(eagine-ecs-private-headers INTERFACE)

target_include_directories(
	eagine-ecs-private-headers
	INTERFACE
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)

foreach(FILENAME ${LIB_INLS})
	set_property(
		TARGET eagine-ecs-private-headers
		APPEND PROPERTY PRIVATE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
	)
endforeach()

