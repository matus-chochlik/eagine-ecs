/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:manipulator;

import std;
import eagine.core.types;
import eagine.core.concepts;

namespace eagine::ecs {

export template <typename Component, bool Const>
class basic_manipulator;
//------------------------------------------------------------------------------
/// @brief Implementation of base functionality for read/write manipulators.
/// @ingroup ecs
/// @note Do not use directly, use manipulator instead.
export template <typename Component>
class basic_manipulator<Component, false> {
public:
    basic_manipulator() noexcept = default;

    basic_manipulator(Component* pcmp) noexcept
      : _ptr{pcmp} {}

    /// @brief Indicates if this manipulator has a value and can read or write.
    [[nodiscard]] auto has_value() const noexcept -> bool {
        return _ptr != nullptr;
    }

    /// @brief Gives read access to the referenced component.
    [[nodiscard]] auto read() const noexcept -> const Component& {
        assert(has_value());
        return *_ptr;
    }

    /// @brief Gives read access to the specified component data member.
    template <typename T>
    [[nodiscard]] auto read(T Component::*const member) const noexcept
      -> optional_reference<const T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    /// @brief Gives write access to the referenced component.
    [[nodiscard]] auto write() const noexcept -> Component& {
        assert(has_value());
        return *_ptr;
    }

    /// @brief Gives write access to the specified component data member.
    template <typename T>
    [[nodiscard]] auto write(T Component::*member) const noexcept
      -> optional_reference<T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    /// @brief Gives write access to the referenced component.
    [[nodiscard]] auto operator->() -> Component* {
        assert(has_value());
        return _ptr;
    }

    /// @brief Calls a function on the referenced component if any.
    /// @see and_then
    template <typename F>
    [[nodiscard]] auto transform(F&& function) const noexcept {
        using R = std::invoke_result_t<F, Component&>;
        if constexpr(std::is_reference_v<R> or std::is_pointer_v<R>) {
            using P = std::conditional_t<
              std::is_reference_v<R>,
              std::remove_reference_t<R>,
              std::remove_pointer_t<R>>;
            if(has_value()) {
                return optional_reference<P>{
                  std::invoke(std::forward<F>(function), this->value())};
            } else {
                return optional_reference<P>{nothing};
            }
        } else {
            using V = std::remove_cvref_t<R>;
            if(has_value()) {
                return std::optional<V>{
                  std::invoke(std::forward<F>(function), this->value())};
            } else {
                return std::optional<V>{};
            }
        }
    }

    /// @brief Calls a function on the referenced component if any.
    /// @see transform
    template <typename F>
        requires(optional_like<
                 std::remove_cvref_t<std::invoke_result_t<F, Component&>>>)
    [[nodiscard]] auto and_then(F&& function) {
        using R = std::remove_cvref_t<std::invoke_result_t<F, Component&>>;
        if(has_value()) {
            return std::invoke(std::forward<F>(function), this->value());
        } else {
            return R{};
        }
    }

protected:
    void _reset_cmp(Component& cmp) noexcept {
        _ptr = &cmp;
    }

private:
    Component* _ptr{nullptr};
};
//------------------------------------------------------------------------------
/// @brief Implementation of base functionality for read-only manipulators.
/// @ingroup ecs
/// @note Do not use directly, use manipulator instead.
export template <typename Component>
class basic_manipulator<Component, true> {
public:
    basic_manipulator() noexcept = default;

    basic_manipulator(const Component* pcmp) noexcept
      : _ptr{pcmp} {}

    /// @brief Indicates if this manipulator has a value and can read or write.
    [[nodiscard]] auto has_value() const noexcept -> bool {
        return _ptr != nullptr;
    }

    /// @brief Gives read access to the specified component data member.
    [[nodiscard]] auto read() const noexcept -> const Component& {
        assert(has_value());
        return *_ptr;
    }

    /// @brief Gives read access to the specified component data member.
    template <typename T>
    [[nodiscard]] auto read(T Component::*const member) const
      -> optional_reference<const T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    /// @brief Gives read access to the referenced component.
    [[nodiscard]] auto operator->() -> const Component* {
        assert(has_value());
        return _ptr;
    }

    /// @brief Calls a function on the referenced component if any.
    /// @see and_then
    template <typename F>
    [[nodiscard]] auto transform(F&& function) const noexcept {
        using R = std::invoke_result_t<F, const Component&>;
        if constexpr(std::is_reference_v<R> or std::is_pointer_v<R>) {
            using P = std::conditional_t<
              std::is_reference_v<R>,
              std::remove_reference_t<R>,
              std::remove_pointer_t<R>>;
            if(has_value()) {
                return optional_reference<P>{
                  std::invoke(std::forward<F>(function), this->value())};
            } else {
                return optional_reference<P>{nothing};
            }
        } else {
            using V = std::remove_cvref_t<R>;
            if(has_value()) {
                return std::optional<V>{
                  std::invoke(std::forward<F>(function), this->value())};
            } else {
                return std::optional<V>{};
            }
        }
    }

    /// @brief Calls a function on the referenced component if any.
    /// @see transform
    template <typename F>
        requires(optional_like<
                 std::remove_cvref_t<std::invoke_result_t<F, const Component&>>>)
    [[nodiscard]] auto and_then(F&& function) {
        using R =
          std::remove_cvref_t<std::invoke_result_t<F, const Component&>>;
        if(has_value()) {
            return std::invoke(std::forward<F>(function), this->value());
        } else {
            return R{};
        }
    }

protected:
    void _reset_cmp(const Component& cmp) noexcept {
        _ptr = &cmp;
    }

private:
    const Component* _ptr{nullptr};
};
//------------------------------------------------------------------------------
export template <typename Component, bool Const>
struct get_manipulator {
    using type = basic_manipulator<Component, Const>;
};

export template <typename Component, bool Const>
using get_manipulator_t = typename get_manipulator<Component, Const>::type;
//------------------------------------------------------------------------------
export template <typename Component>
class manipulator
  : public get_manipulator_t<
      std::remove_const_t<Component>,
      std::is_const_v<Component>> {
    using _nonconstC = std::remove_const_t<Component>;
    _nonconstC* _add_place{nullptr};

public:
    manipulator() noexcept = default;

    manipulator(const bool can_rem) noexcept
      : _can_rem{can_rem} {}

    manipulator(Component* pcmp, const bool can_rem) noexcept
      : _base{pcmp}
      , _can_rem{can_rem} {}

    manipulator(Component& cmp, const bool can_rem) noexcept
      : _base{&cmp}
      , _can_rem{can_rem} {}

    manipulator(Component& cmp, _nonconstC& add, const bool can_rem) noexcept
      : _base(&cmp)
      , _add_place{&add}
      , _can_rem(can_rem) {}

    manipulator(std::nullptr_t, _nonconstC& add, const bool can_rem) noexcept
      : _add_place{&add}
      , _can_rem{can_rem} {}

    [[nodiscard]] auto can_add_component() const noexcept -> bool {
        return _add_place != nullptr;
    }

    void add_component(std::remove_const_t<Component>&& cmp) {
        assert(can_add_component());
        assert(_add_place);
        *_add_place = std::move(cmp);
        this->_reset_cmp(*_add_place);
        _added = true;
    }

    [[nodiscard]] auto can_remove() const noexcept -> bool {
        return _can_rem and this->has_value();
    }

    void remove() noexcept {
        _removed = true;
    }

protected:
    const bool _can_rem{false};
    bool _removed{false};
    bool _added{false};

private:
    using _base = get_manipulator_t<
      std::remove_const_t<Component>,
      std::is_const_v<Component>>;
};
//------------------------------------------------------------------------------
export template <typename Component>
class concrete_manipulator : public manipulator<Component> {
public:
    using manipulator<Component>::manipulator;

    void reset(Component& cmp) noexcept {
        this->_reset_cmp(cmp);
        this->_added = false;
        this->_removed = false;
    }

    [[nodiscard]] auto add_requested() const noexcept -> bool {
        return this->_added;
    }

    [[nodiscard]] auto remove_requested() const noexcept -> bool {
        return this->_removed;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

