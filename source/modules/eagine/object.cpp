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
/// @brief Default instantiation of basic_manager with the default entity type.
/// @ingroup ecs
/// @see eagine::enable_ecs
export using default_manager = basic_manager<identifier_value>;
//------------------------------------------------------------------------------
/// @brief Class providing access to default ECS manager instance.
/// @ingroup main_context
/// @note Do not use directly, use either @c object or @c locate_default_manager.
/// @relates default_manager
class default_manager_holder
  : public main_ctx_service_impl<default_manager_holder>
  , public main_ctx_object {
public:
    default_manager_holder(main_ctx_parent parent) noexcept;

    static auto static_type_id() noexcept -> identifier;

    /// @brief Optionally returns the reference to the main context ECS manager instance.
    auto get() noexcept -> optional_reference<default_manager> {
        return _manager;
    }

private:
    default_manager _manager{};
};
//------------------------------------------------------------------------------
/// @brief Function locating the @c default_manager instance from @c main_ctx.
/// @ingroup ecs
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
/// @brief Wrapper around entity from the @c default_manger
/// @ingroup ecs
/// @see default_manager
export class object {
public:
    /// @brief Type alias for the default entity type used by the default manager.
    using entity_type = identifier_value;

    /// @brief Constructor specifying the identifier of the constructed object.
    /// @post this->entity() == id
    object(identifier_value id) noexcept;

    /// @brief Move constructor.
    object(object&&) noexcept = default;

    object(const object&) = delete;

    /// @brief Move assignment.
    auto operator=(object&&) noexcept -> object& = default;

    auto operator=(const object&) = delete;
    ~object() noexcept;

    /// @brief Spawns a new object by generating a unique entity / id.
    [[nodiscard]] static auto spawn(main_ctx& ctx) noexcept -> object;

    /// @brief Spawns a new object by generating a unique entity / id.
    [[nodiscard]] static auto spawn(const main_ctx_object& parent) noexcept
      -> object;

    /// @brief Returns the underlying entity of this object.
    [[nodiscard]] auto entity() const noexcept -> identifier_value {
        return _id;
    }

    /// @brief Returns the reference to the parent manager of this object,
    [[nodiscard]] auto manager() const noexcept -> default_manager& {
        return *locate_default_manager();
    }

    /// @brief Indicates if this object has the specified @c Component.
    /// @see has_all
    /// @see add
    /// @see ensure
    template <component_data Component>
    [[nodiscard]] auto has() noexcept -> bool {
        return manager().template has<Component>(entity());
    }

    /// @brief Indicates if this object has all the specified @c Components.
    /// @see has
    /// @see add
    /// @see ensure
    template <component_data... Components>
    [[nodiscard]] auto has_all() -> bool {
        return manager().template has_all<Components...>(entity());
    }

    /// @brief Indicates if this object has the specified @c Component hidden.
    /// @see hide
    /// @see show
    template <component_data Component>
    [[nodiscard]] auto is_hidden() noexcept -> bool {
        return manager().template is_hidden<Component>(entity());
    }

    /// @brief Shows the specified @c Components of this object.
    /// @see is_hidden
    /// @see show
    template <component_data... Components>
    auto show() noexcept -> object& {
        manager().template show<Components...>(entity());
        return *this;
    }

    /// @brief Hides the specified @c Components of this object.
    /// @see is_hidden
    /// @see hide
    template <component_data... Components>
    auto hide() noexcept -> object& {
        manager().template hide<Components...>(entity());
        return *this;
    }

    /// @brief Adds the specified component instances to this object.
    /// @see ensure
    /// @see has
    /// @see has_all
    template <component_data... Components>
    auto add(Components&&... components) -> auto& {
        return manager().add(entity(), std::forward<Components>(components)...);
    }

    /// @brief Ensures that this object has an instance of the specified @c Component.
    /// @see ensure
    /// @see has
    /// @see has_all
    template <component_data Component>
    auto ensure(std::type_identity<Component> tid = {})
      -> manipulator<Component> {
        return manager().ensure(entity(), tid);
    }

    /// @brief Copies the instances of the specified components from another entity.
    /// @see exchange_with
    template <component_data Component>
    auto copy_from(identifier_value from) -> manipulator<Component> {
        return manager().template copy<Component>(from, entity());
    }

    /// @brief Copies the instances of the specified components from another object.
    /// @see exchange_with
    template <component_data Component>
    auto copy_from(const object& from) -> manipulator<Component> {
        return copy_from<Component>(from.entity());
    }

    /// @brief Exchanges the instances of the specified components with another entity.
    /// @see copy_from
    template <component_data Component>
    auto exchange_with(identifier_value that) -> object& {
        manager().template exchange<Component>(entity(), that);
        return *this;
    }

    /// @brief Exchanges the instances of the specified components with another entity.
    /// @see copy_from
    template <component_data Component>
    auto exchange_with(const object& that) -> object& {
        return exchange_with<Component>(that.entity());
    }

    /// @brief Calls specified function on the read manipulator of the @c Component.
    /// @see write
    template <component_data Component, typename Function>
    auto read(Function&& function) -> auto& {
        return manager().template read_single<Component>(
          entity(), std::forward<Function>(function));
    }

    /// @brief Calls specified function on the write manipulator of the @c Component.
    /// @see read
    template <component_data Component, typename Function>
    auto write(Function&& function) -> auto& {
        return manager().template write_single<Component>(
          entity(), std::forward<Function>(function));
    }

    /// @brief Removes the specified @c Components from this object.
    /// @see forget
    /// @see has
    /// @see has_all
    template <component_data... Components>
    auto remove() noexcept -> object& {
        manager().template remove<Components...>(entity());
        return *this;
    }

    /// @brief Removes all components of this object from the parent manager.
    /// @see remove
    /// @see has
    /// @see has_all
    auto forget() noexcept -> object&;

private:
    identifier_value _id;
};
//------------------------------------------------------------------------------
} // namespace ecs
//------------------------------------------------------------------------------
/// @brief Enables the @c ecs::default_manager in @c main_ctx and the use of ECS objects.
/// @ingroup main_ctx
/// @see ecs::default_manager
/// @see ecs::object
export auto enable_ecs(main_ctx&) -> optional_reference<ecs::default_manager>;
//------------------------------------------------------------------------------
} // namespace eagine
