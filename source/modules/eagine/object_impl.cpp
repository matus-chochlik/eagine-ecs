/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.ecs;

import std;
import eagine.core;

namespace eagine {
namespace ecs {
//------------------------------------------------------------------------------
// default_manager_holder
//------------------------------------------------------------------------------
default_manager_holder::default_manager_holder(main_ctx_parent parent) noexcept
  : main_ctx_object{"ECSManager", parent} {}
//------------------------------------------------------------------------------
auto default_manager_holder::static_type_id() noexcept -> identifier {
    return "ECSManager";
}
//------------------------------------------------------------------------------
auto enable(main_ctx& ctx) -> optional_reference<default_manager> {
    assert(ctx.setters());
    return ctx.setters().and_then([&](auto& setters) {
        shared_holder<default_manager_holder> ecs_mgr{default_selector, ctx};
        auto mgr_ref{ecs_mgr->get()};
        setters.inject(std::move(ecs_mgr));
        return mgr_ref;
    });
}
//------------------------------------------------------------------------------
// object
//------------------------------------------------------------------------------
auto object::spawn(main_ctx& ctx) noexcept -> object {
    auto mgr{locate_default_manager(ctx)};
    assert(mgr);
    return object{mgr->spawn()};
}
//------------------------------------------------------------------------------
auto object::spawn(const main_ctx_object& parent) noexcept -> object {
    auto mgr{default_manager_of(parent)};
    assert(mgr);
    return object{mgr->spawn()};
}
//------------------------------------------------------------------------------
object::object(identifier_value id) noexcept
  : _id{id} {
    assert(not manager().knows(entity()));
}
//------------------------------------------------------------------------------
object::~object() noexcept {
    manager().forget(entity());
}
//------------------------------------------------------------------------------
auto object::forget() noexcept -> object& {
    manager().forget(entity());
    return *this;
}
//------------------------------------------------------------------------------
} // namespace ecs
//------------------------------------------------------------------------------
auto enable_ecs(main_ctx& ctx) -> optional_reference<ecs::default_manager> {
    return ecs::enable(ctx);
}
//------------------------------------------------------------------------------
} // namespace eagine
