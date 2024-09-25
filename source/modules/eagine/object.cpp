/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
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
export using default_manager = basic_manager<identifier_value>;
//------------------------------------------------------------------------------
/// @brief Class providing access to default ECS manager instance.
/// @ingroup main_context
class default_manager_holder
  : public main_ctx_service_impl<default_manager_holder>
  , public main_ctx_object {
public:
    default_manager_holder(main_ctx_parent parent) noexcept;

    static auto static_type_id() noexcept -> identifier;

    auto get() noexcept -> optional_reference<default_manager> {
        return _manager;
    }

private:
    default_manager _manager{};
};
//------------------------------------------------------------------------------
export auto locate_default_manager(main_ctx& ctx) noexcept
  -> optional_reference<default_manager> {
    return ctx.locate<default_manager_holder>().and_then(
      [](auto& holder) { return holder.get(); });
}

auto locate_default_manager() noexcept -> optional_reference<default_manager> {
    return locate_default_manager(main_ctx::get());
}

auto default_manager_of(const main_ctx_object& user) noexcept
  -> optional_reference<default_manager> {
    return user.locate<default_manager_holder>().and_then(
      [](auto& holder) { return holder.get(); });
}
//------------------------------------------------------------------------------
export class object {
public:
    using entity_type = identifier_value;

    object(identifier_value id) noexcept;
    object(object&&) noexcept = default;
    object(const object&) = delete;
    auto operator=(object&&) noexcept -> object& = default;
    auto operator=(const object&) = delete;
    ~object() noexcept;

    [[nodiscard]] static auto spawn(main_ctx& ctx) noexcept -> object;

    [[nodiscard]] static auto spawn(const main_ctx_object& parent) noexcept
      -> object;

    [[nodiscard]] auto entity() const noexcept -> identifier_value {
        return _id;
    }

    [[nodiscard]] auto manager() const noexcept -> default_manager& {
        return *locate_default_manager();
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
    auto copy_from(identifier_value from) -> manipulator<Component> {
        return manager().template copy<Component>(from, entity());
    }

    template <component_data Component>
    auto copy_from(const object& from) -> manipulator<Component> {
        return copy_from<Component>(from.entity());
    }

    template <component_data Component>
    auto exchange_with(identifier_value that) -> object& {
        manager().template exchange<Component>(entity(), that);
        return *this;
    }

    template <component_data Component>
    auto exchange_with(const object& that) -> object& {
        return exchange_with<Component>(that.entity());
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

    auto forget() noexcept -> object&;

private:
    identifier_value _id;
};
//------------------------------------------------------------------------------
} // namespace ecs
//------------------------------------------------------------------------------
export auto enable_ecs(main_ctx&) -> optional_reference<ecs::default_manager>;
//------------------------------------------------------------------------------
} // namespace eagine
