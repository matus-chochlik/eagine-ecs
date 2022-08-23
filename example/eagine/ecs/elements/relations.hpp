/// @example eagine/ecs/elements/relations.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_EXAMPLE_ECS_RELATIONS_HPP // NOLINT(llvm-header-guard)
#define EAGINE_EXAMPLE_ECS_RELATIONS_HPP

#include "entity.hpp"
#if !EAGINE_ECS_MODULE
#include <eagine/ecs/component.hpp>
#include <eagine/ecs/manipulator.hpp>
#include <eagine/identifier.hpp>
#endif

namespace eagine {
//------------------------------------------------------------------------------
struct isotope : ecs::relation<"Isotope"> {};
//------------------------------------------------------------------------------
} // namespace eagine

#endif
