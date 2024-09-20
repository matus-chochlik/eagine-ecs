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

namespace eagine::ecs {
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
} // namespace eagine::ecs
