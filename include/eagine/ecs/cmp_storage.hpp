/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_ECS_CMP_STORAGE_HPP
#define EAGINE_ECS_CMP_STORAGE_HPP

#include "entity_traits.hpp"
#include "manipulator.hpp"
#include "storage_caps.hpp"
#include "storage_fwd.hpp"
#include <eagine/assert.hpp>
#include <eagine/callable_ref.hpp>
#include <eagine/interface.hpp>
#include <cassert>

namespace eagine::ecs {
//------------------------------------------------------------------------------
template <typename Entity>
struct storage_iterator_intf<Entity, false>
  : interface<storage_iterator_intf<Entity, false>> {

    virtual void reset() = 0;

    virtual auto done() -> bool = 0;

    virtual void next() = 0;

    virtual auto find(Entity) -> bool = 0;

    virtual auto current() -> Entity = 0;
};
//------------------------------------------------------------------------------
template <typename Entity>
class storage_iterator<Entity, false> {
public:
    storage_iterator(storage_iterator_intf<Entity, false>* i) noexcept
      : _i{i} {
        EAGINE_ASSERT(_i);
    }

    storage_iterator(storage_iterator&& tmp) noexcept
      : _i{std::exchange(tmp._i, nullptr)} {}

    storage_iterator(const storage_iterator&) = delete;
    auto operator=(storage_iterator&&) = delete;
    auto operator=(const storage_iterator&) = delete;

    ~storage_iterator() noexcept {
        EAGINE_ASSERT(_i == nullptr);
    }

    auto release() -> storage_iterator_intf<Entity, false>* {
        return std::exchange(_i, nullptr);
    }

    auto ptr() noexcept -> storage_iterator_intf<Entity, false>* {
        EAGINE_ASSERT(_i);
        return _i;
    }

    auto get() noexcept -> storage_iterator_intf<Entity, false>& {
        EAGINE_ASSERT(_i);
        return *_i;
    }

    void reset() {
        get().reset();
    }

    auto done() -> bool {
        return get().done();
    }

    auto next() -> auto& {
        get().next();
        return *this;
    }

    auto find(Entity e) -> bool {
        return get().find(e);
    }

    auto current() -> Entity {
        return get().current();
    }

private:
    storage_iterator_intf<Entity, false>* _i{nullptr};
};
//------------------------------------------------------------------------------
template <typename Entity>
struct base_storage<Entity, false> : interface<base_storage<Entity, false>> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, false>;

    virtual auto capabilities() -> storage_caps = 0;

    virtual auto new_iterator() -> iterator_t = 0;

    virtual void delete_iterator(iterator_t&&) = 0;

    virtual auto has(entity_param) -> bool = 0;

    virtual auto is_hidden(entity_param) -> bool = 0;

    virtual auto is_hidden(iterator_t&) -> bool = 0;

    virtual auto hide(entity_param) -> bool = 0;

    virtual void hide(iterator_t&) = 0;

    virtual auto show(entity_param) -> bool = 0;

    virtual auto show(iterator_t&) -> bool = 0;

    virtual auto copy(entity_param from, entity_param to) -> void* = 0;

    virtual auto swap(entity_param a, entity_param b) -> bool = 0;

    virtual auto remove(entity_param) -> bool = 0;

    virtual void remove(iterator_t&) = 0;
};
//------------------------------------------------------------------------------
template <typename Entity, typename Component>
struct storage<Entity, Component, false> : base_storage<Entity, false> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, false>;

    virtual auto store(entity_param, Component&&) -> Component* = 0;

    virtual auto store(iterator_t&, entity_param, Component&&)
      -> Component* = 0;

    virtual void for_single(
      callable_ref<void(entity_param, manipulator<const Component>&)>,
      entity_param) = 0;

    virtual void for_single(
      callable_ref<void(entity_param, manipulator<const Component>&)>,
      iterator_t&) = 0;

    virtual void for_single(
      callable_ref<void(entity_param, manipulator<Component>&)>,
      entity_param) = 0;

    virtual void for_single(
      callable_ref<void(entity_param, manipulator<Component>&)>,
      iterator_t&) = 0;

    virtual void for_each(
      callable_ref<void(entity_param, manipulator<const Component>&)>) = 0;

    virtual void for_each(
      callable_ref<void(entity_param, manipulator<Component>&)>) = 0;
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

#endif // EAGINE_ECS_CMP_STORAGE_HPP
