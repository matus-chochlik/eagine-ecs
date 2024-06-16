#  Copyright Matus Chochlik.
#  Distributed under the Boost Software License, Version 1.0.
#  See accompanying file LICENSE_1_0.txt or copy at
#  https://www.boost.org/LICENSE_1_0.txt
#
# Package specific options
#  Debian
#   Dependencies
set(CXX_RUNTIME_PKGS "libc6,libc++1-17")
set(CPACK_DEBIAN_ECS-DEV_PACKAGE_DEPENDS "eagine-core-dev (>= @EAGINE_VERSION@)")
set(CPACK_DEBIAN_ECS-EXAMPLES_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS}")
#   Descriptions
set(CPACK_DEBIAN_ECS-DEV_DESCRIPTION "C++ entity-component system.")
set(CPACK_DEBIAN_ECS-EXAMPLES_DESCRIPTION "EAGine ECS examples.")

