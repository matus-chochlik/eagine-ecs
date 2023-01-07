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

import eagine.core.types;
import <type_traits>;
import <utility>;

namespace eagine::ecs {

export template <typename Component, bool Const>
class basic_manipulator;
//------------------------------------------------------------------------------
export template <typename Component>
class basic_manipulator<Component, false> {
public:
    basic_manipulator() noexcept = default;

    basic_manipulator(Component* pcmp) noexcept
      : _ptr{pcmp} {}

    [[nodiscard]] auto has_value() const noexcept -> bool {
        return _ptr != nullptr;
    }

    [[nodiscard]] auto read() const noexcept -> const Component& {
        assert(has_value());
        return *_ptr;
    }

    template <typename T>
    [[nodiscard]] auto read(T Component::*const member) const noexcept
      -> optional_reference<const T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    [[nodiscard]] auto write() -> Component& {
        assert(has_value());
        return *_ptr;
    }

    template <typename T>
    [[nodiscard]] auto write(T Component::*member) const noexcept
      -> optional_reference<T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    [[nodiscard]] auto operator->() -> Component* {
        assert(has_value());
        return _ptr;
    }

protected:
    void _reset_cmp(Component& cmp) noexcept {
        _ptr = &cmp;
    }

private:
    Component* _ptr{nullptr};
};
//------------------------------------------------------------------------------
export template <typename Component>
class basic_manipulator<Component, true> {
public:
    basic_manipulator() noexcept = default;

    basic_manipulator(const Component* pcmp) noexcept
      : _ptr{pcmp} {}

    [[nodiscard]] auto has_value() const noexcept -> bool {
        return _ptr != nullptr;
    }

    [[nodiscard]] auto read() const noexcept -> const Component& {
        assert(has_value());
        return *_ptr;
    }

    template <typename T>
    [[nodiscard]] auto read(T Component::*const member) const
      -> optional_reference<const T> {
        if(_ptr != nullptr) {
            return {*_ptr.*member};
        }
        return {nothing};
    }

    [[nodiscard]] auto operator->() -> const Component* {
        assert(has_value());
        return _ptr;
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
    manipulator() = default;

    manipulator(const bool can_rem)
      : _can_rem{can_rem} {}

    manipulator(Component* pcmp, const bool can_rem)
      : _base{pcmp}
      , _can_rem{can_rem} {}

    manipulator(Component& cmp, const bool can_rem)
      : _base{&cmp}
      , _can_rem{can_rem} {}

    manipulator(Component& cmp, _nonconstC& add, const bool can_rem)
      : _base(&cmp)
      , _add_place{&add}
      , _can_rem(can_rem) {}

    manipulator(std::nullptr_t, _nonconstC& add, const bool can_rem)
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

    void remove() {
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

