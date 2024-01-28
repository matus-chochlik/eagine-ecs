/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:object;

import std;
import eagine.core.types;
import eagine.core.identifier;
import eagine.core.main_ctx;
import :component;
import :manipulator;
import :manager;

namespace eagine {
namespace ecs {
//------------------------------------------------------------------------------
export using default_manager = basic_manager<identifier>;
//------------------------------------------------------------------------------
/// @brief Class providing access to default ECS manager instance.
/// @ingroup main_context
class default_manager_holder
  : public main_ctx_service_impl<default_manager_holder>
  , public main_ctx_object {
public:
    default_manager_holder(main_ctx_parent parent) noexcept
      : main_ctx_object{"ECSManager", parent} {}

    static constexpr auto static_type_id() noexcept -> identifier {
        return "ECSManager";
    }

    auto get() noexcept -> optional_reference<default_manager> {
        return _manager;
    }

private:
    default_manager _manager{};
};
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

export auto locate_default_manager(main_ctx& ctx) noexcept
  -> optional_reference<default_manager> {
    return ctx.locate<default_manager_holder>().and_then(
      [](auto& holder) { return holder.get(); });
}

auto default_manager_of(const main_ctx_object& user) noexcept
  -> optional_reference<default_manager> {
    return user.locate<default_manager_holder>().and_then(
      [](auto& holder) { return holder.get(); });
}
//------------------------------------------------------------------------------
export class object : public main_ctx_object {
public:
    object(identifier id, main_ctx_parent parent) noexcept
      : main_ctx_object{id, parent} {
        assert(not manager().knows(entity()));
    }

    object(object&&) noexcept = default;
    object(const object&) = delete;
    auto operator=(object&&) noexcept -> object& = default;
    auto operator=(const object&) = delete;
    ~object() noexcept {
        manager().forget(entity());
    }

    [[nodiscard]] static auto spawn(main_ctx& ctx) noexcept -> object {
        auto mgr{locate_default_manager(ctx)};
        assert(mgr);
        return object{mgr->spawn(), ctx};
    }

    [[nodiscard]] static auto spawn(const main_ctx_object& parent) noexcept
      -> object {
        auto mgr{default_manager_of(parent)};
        assert(mgr);
        return object{mgr->spawn(), parent};
    }

    auto entity() const noexcept -> identifier {
        return object_id();
    }

    auto manager() const noexcept -> default_manager& {
        // TODO: optimization: static variable - pointer
        return *_locate_manager();
    }

    template <component_data Component>
    [[nodiscard]] auto has() noexcept -> bool {
        return manager().template has<Component>(entity());
    }

    template <component_data... Components>
    [[nodiscard]] auto has_all() -> bool {
        return manager().template has_all<Components...>(entity());
    }

    template <component_data Component>
    [[nodiscard]] auto is_hidden() noexcept -> bool {
        return manager().template is_hidden<Component>(entity());
    }

    template <component_data... Components>
    auto show() noexcept -> object& {
        manager().template show<Components...>(entity());
        return *this;
    }

    template <component_data... Components>
    auto hide() noexcept -> object& {
        manager().template hide<Components...>(entity());
        return *this;
    }

    template <component_data... Components>
    auto add(Components&&... components) -> auto& {
        return manager().add(entity(), std::forward<Components>(components)...);
    }

    template <component_data Component>
    auto ensure(std::type_identity<Component> tid = {})
      -> manipulator<Component> {
        return manager().ensure(entity(), tid);
    }

    template <component_data Component>
    auto copy_from(identifier from) -> manipulator<Component> {
        return manager().template copy<Component>(from, entity());
    }

    template <component_data Component>
    auto copy_from(const object& from) -> manipulator<Component> {
        return copy_from<Component>(from.entity());
    }

    template <component_data Component>
    auto swap_with(identifier that) -> manipulator<Component> {
        return manager().template swap<Component>(entity(), that);
    }

    template <component_data Component>
    auto swap_with(const object& that) -> manipulator<Component> {
        return swap_with<Component>(that.entity());
    }

    template <component_data Component, typename Function>
    auto read(Function&& function) -> auto& {
        return manager().template read_single<Component>(
          entity(), std::forward<Function>(function));
    }

    template <component_data Component, typename Function>
    auto write(Function&& function) -> auto& {
        return manager().template write_single<Component>(
          entity(), std::forward<Function>(function));
    }

    template <component_data... Components>
    auto remove() noexcept -> object& {
        manager().template remove<Components...>(entity());
        return *this;
    }

    auto forget() noexcept -> object& {
        manager().forget(entity());
        return *this;
    }

private:
    auto _locate_manager() const noexcept
      -> optional_reference<default_manager> {
        return default_manager_of(*this);
    }
};
//------------------------------------------------------------------------------
} // namespace ecs
//------------------------------------------------------------------------------
export auto enable_ecs(main_ctx& ctx)
  -> optional_reference<ecs::default_manager> {
    return ecs::enable(ctx);
}
//------------------------------------------------------------------------------
} // namespace eagine
