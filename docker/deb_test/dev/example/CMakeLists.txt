# Copyright Matus Chochlik.
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# https://www.boost.org/LICENSE_1_0.txt
#
add_executable(eagine-space_opera space_opera.cpp)
target_link_libraries(eagine-space_opera PUBLIC EAGine::ECS)

