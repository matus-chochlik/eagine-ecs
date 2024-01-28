/// @example eagine/ecs/elements/relations.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_EXAMPLE_ECS_RELATIONS_HPP // NOLINT(llvm-header-guard)
#define EAGINE_EXAMPLE_ECS_RELATIONS_HPP

#include "entity.hpp"

namespace eagine {
//------------------------------------------------------------------------------
struct isotope : ecs::relation<"Isotope"> {};
//------------------------------------------------------------------------------
} // namespace eagine

#endif
