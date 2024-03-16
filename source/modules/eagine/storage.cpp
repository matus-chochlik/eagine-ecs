/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:storage;

import std;
import eagine.core.types;
import eagine.core.reflection;
import eagine.core.utility;
import :entity_traits;
import :manipulator;

namespace eagine::ecs {
//------------------------------------------------------------------------------
//  Storage buffer
//------------------------------------------------------------------------------
export enum class storage_buffer : bool {
    next = false,
    current = true,
    write = false,
    read = true
};
//------------------------------------------------------------------------------
constexpr auto storage_buffer_from_constness(bool is_const) noexcept
  -> storage_buffer {
    return static_cast<storage_buffer>(is_const);
}
//------------------------------------------------------------------------------
//  Capabilities
//------------------------------------------------------------------------------
export enum class storage_cap_bit : unsigned short {
    double_buffer = 1U << 0U,
    hide = 1U << 1U,
    copy = 1U << 2U,
    exchange = 1U << 3U,
    store = 1U << 4U,
    remove = 1U << 5U,
    modify = 1U << 6U
};
//------------------------------------------------------------------------------
export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<storage_cap_bit>,
  const Selector) noexcept {
    return enumerator_map_type<storage_cap_bit, 7>{
      {{"double_buffer", storage_cap_bit::double_buffer},
       {"hide", storage_cap_bit::hide},
       {"copy", storage_cap_bit::copy},
       {"exchange", storage_cap_bit::exchange},
       {"store", storage_cap_bit::store},
       {"remove", storage_cap_bit::remove},
       {"modify", storage_cap_bit::modify}}};
}
//------------------------------------------------------------------------------
export [[nodiscard]] auto operator|(
  const storage_cap_bit a,
  const storage_cap_bit b) noexcept -> bitfield<storage_cap_bit> {
    return {a, b};
}
//------------------------------------------------------------------------------
export class storage_caps : public bitfield<storage_cap_bit> {
    using _base = bitfield<storage_cap_bit>;

public:
    storage_caps() noexcept = default;

    storage_caps(const bitfield<storage_cap_bit> base)
      : _base{base} {}

    [[nodiscard]] auto can_swap_buffers() const noexcept -> bool {
        return has(storage_cap_bit::double_buffer);
    }

    [[nodiscard]] auto can_hide() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    [[nodiscard]] auto can_copy() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    [[nodiscard]] auto can_exchange() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    [[nodiscard]] auto can_remove() const noexcept -> bool {
        return has(storage_cap_bit::remove);
    }

    [[nodiscard]] auto can_store() const noexcept -> bool {
        return has(storage_cap_bit::store);
    }

    [[nodiscard]] auto can_modify() const noexcept -> bool {
        return has(storage_cap_bit::modify);
    }
};
//------------------------------------------------------------------------------
export auto all_storage_caps() noexcept -> storage_caps {
    return {static_cast<storage_cap_bit>((1U << 7U) - 1U)};
}
//------------------------------------------------------------------------------
// Signals
//------------------------------------------------------------------------------
export template <typename Entity>
struct basic_manager_signals {
    signal<void(entity_param_t<Entity>) noexcept> entity_spawned;
    signal<void(entity_param_t<Entity>) noexcept> entity_forgotten;
};
//------------------------------------------------------------------------------
// Interfaces
//------------------------------------------------------------------------------
export template <typename Entity, data_kind>
struct storage_iterator_intf;

export template <typename Entity>
using component_storage_iterator_intf =
  storage_iterator_intf<Entity, data_kind::component>;

export template <typename Entity>
using relation_storage_iterator_intf =
  storage_iterator_intf<Entity, data_kind::relation>;

export template <typename Entity, data_kind>
class storage_iterator;

export template <typename Entity>
using component_storage_iterator =
  storage_iterator<Entity, data_kind::component>;

export template <typename Entity>
using relation_storage_iterator = storage_iterator<Entity, data_kind::relation>;

export template <typename Entity, data_kind>
struct base_storage;

export template <typename Entity>
using base_component_storage = base_storage<Entity, data_kind::component>;

export template <typename Entity>
using base_relation_storage = base_storage<Entity, data_kind::relation>;

export template <typename Entity, typename Data, data_kind>
struct storage;

export template <typename Entity, typename Data>
using component_storage = storage<Entity, Data, data_kind::component>;

export template <typename Entity, typename Data>
using relation_storage = storage<Entity, Data, data_kind::relation>;
//------------------------------------------------------------------------------
//  Component storage
//------------------------------------------------------------------------------
export template <typename Entity>
struct storage_iterator_intf<Entity, data_kind::component>
  : abstract<storage_iterator_intf<Entity, data_kind::component>> {

    virtual void reset() = 0;

    virtual auto done() -> bool = 0;

    virtual void next() = 0;

    virtual auto find(entity_param_t<Entity>) -> bool = 0;

    virtual auto current() -> Entity = 0;
};
//------------------------------------------------------------------------------
export template <typename Entity>
class storage_iterator<Entity, data_kind::component> {
public:
    storage_iterator(
      storage_iterator_intf<Entity, data_kind::component>& i) noexcept
      : _i{&i} {}

    storage_iterator(
      storage_iterator_intf<Entity, data_kind::component>* i) noexcept
      : _i{i} {
        assert(_i);
    }

    storage_iterator(storage_iterator&& tmp) noexcept
      : _i{std::exchange(tmp._i, nullptr)} {}

    storage_iterator(const storage_iterator&) = delete;
    auto operator=(storage_iterator&&) = delete;
    auto operator=(const storage_iterator&) = delete;

    ~storage_iterator() noexcept {
        assert(_i == nullptr);
    }

    auto release() -> storage_iterator_intf<Entity, data_kind::component>* {
        return std::exchange(_i, nullptr);
    }

    auto ptr() noexcept
      -> storage_iterator_intf<Entity, data_kind::component>* {
        assert(_i);
        return _i;
    }

    auto get() noexcept
      -> storage_iterator_intf<Entity, data_kind::component>& {
        assert(_i);
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

    auto find(entity_param_t<Entity> e) -> bool {
        return get().find(e);
    }

    auto current() -> Entity {
        return get().current();
    }

private:
    storage_iterator_intf<Entity, data_kind::component>* _i{nullptr};
};
//------------------------------------------------------------------------------
export template <typename Entity>
struct base_storage<Entity, data_kind::component>
  : interface<base_storage<Entity, data_kind::component>> {
    using entity_param = entity_param_t<Entity>;
    using signals_t = basic_manager_signals<Entity>;
    using iterator_t = storage_iterator<Entity, data_kind::component>;

    virtual auto capabilities() -> storage_caps = 0;

    virtual void swap_buffers() = 0;

    virtual auto new_iterator(storage_buffer) -> iterator_t = 0;

    virtual void delete_iterator(iterator_t&&) = 0;

    virtual auto has(entity_param) -> bool = 0;

    virtual auto is_hidden(entity_param) -> bool = 0;

    virtual auto is_hidden(iterator_t&) -> bool = 0;

    virtual auto hide(entity_param) -> bool = 0;

    virtual void hide(iterator_t&) = 0;

    virtual auto show(entity_param) -> bool = 0;

    virtual auto show(iterator_t&) -> bool = 0;

    virtual auto copy(entity_param from, entity_param to) -> void* = 0;

    virtual auto exchange(entity_param a, entity_param b) -> bool = 0;

    virtual auto remove(entity_param) -> bool = 0;

    virtual void remove(iterator_t&) = 0;
};
//------------------------------------------------------------------------------
export template <typename Entity, typename Component>
struct storage<Entity, Component, data_kind::component>
  : base_storage<Entity, data_kind::component> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, data_kind::component>;

    virtual auto store(entity_param, Component&&) -> Component* = 0;

    virtual auto store(iterator_t&, entity_param, Component&&)
      -> Component* = 0;

    virtual void for_single(
      const callable_ref<void(entity_param, manipulator<const Component>&)>,
      entity_param) = 0;

    virtual void for_single(
      const callable_ref<void(entity_param, manipulator<const Component>&)>,
      iterator_t&) = 0;

    virtual void for_single(
      const callable_ref<void(entity_param, manipulator<Component>&)>,
      entity_param) = 0;

    virtual void for_single(
      const callable_ref<void(entity_param, manipulator<Component>&)>,
      iterator_t&) = 0;

    virtual void for_each(
      const callable_ref<void(entity_param, manipulator<const Component>&)>) = 0;

    virtual void for_each(
      const callable_ref<void(entity_param, manipulator<Component>&)>) = 0;

    virtual void for_each(const callable_ref<void(manipulator<Component>&)>) = 0;
};
//------------------------------------------------------------------------------
//  Relation storage
//------------------------------------------------------------------------------
export template <typename Entity>
struct storage_iterator_intf<Entity, data_kind::relation>
  : abstract<storage_iterator_intf<Entity, data_kind::relation>> {

    virtual auto reset() -> void = 0;

    virtual auto done() -> bool = 0;

    virtual void next() = 0;

    virtual auto subject() -> Entity = 0;

    virtual auto object() -> Entity = 0;
};
//------------------------------------------------------------------------------
export template <typename Entity>
class storage_iterator<Entity, data_kind::relation> {
public:
    storage_iterator(
      storage_iterator_intf<Entity, data_kind::relation>& i) noexcept
      : _i{&i} {}

    storage_iterator(
      storage_iterator_intf<Entity, data_kind::relation>* i) noexcept
      : _i{i} {
        assert(_i);
    }

    storage_iterator(storage_iterator&& tmp) noexcept
      : _i{std::exchange(tmp._i, nullptr)} {}
    storage_iterator(const storage_iterator&) = delete;

    auto operator=(storage_iterator&&) = delete;
    auto operator=(const storage_iterator&) = delete;

    ~storage_iterator() noexcept {
        assert(_i == nullptr);
    }

    auto release() -> storage_iterator_intf<Entity, data_kind::relation>* {
        return std::exchange(_i, nullptr);
    }

    auto ptr() noexcept -> storage_iterator_intf<Entity, data_kind::relation>* {
        assert(_i);
        return _i;
    }

    auto get() noexcept -> storage_iterator_intf<Entity, data_kind::relation>& {
        assert(_i);
        return *_i;
    }

    void reset() {
        get().reset();
    }

    auto done() -> bool {
        return get().done();
    }

    void next() {
        get().next();
    }

    auto subject() -> Entity {
        return get().subject();
    }

    auto object() -> Entity {
        return get().object();
    }

private:
    storage_iterator_intf<Entity, data_kind::relation>* _i{nullptr};
};
//------------------------------------------------------------------------------
export template <typename Entity>
struct base_storage<Entity, data_kind::relation>
  : interface<base_storage<Entity, data_kind::relation>> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, data_kind::relation>;

    virtual auto capabilities() -> storage_caps = 0;

    virtual void swap_buffers() = 0;

    virtual auto new_iterator(storage_buffer) -> iterator_t = 0;

    virtual void delete_iterator(iterator_t&&) = 0;

    virtual auto has(entity_param subject, entity_param object) -> bool = 0;

    virtual auto store(entity_param subject, entity_param object) -> bool = 0;

    virtual auto remove(entity_param subject, entity_param object) -> bool = 0;

    virtual void remove(iterator_t&) = 0;

    virtual void for_each(
      const callable_ref<void(entity_param, entity_param)>,
      entity_param subject) = 0;

    virtual void for_each(callable_ref<void(entity_param, entity_param)>) = 0;
};
//------------------------------------------------------------------------------
export template <typename Entity, typename Relation>
struct storage<Entity, Relation, data_kind::relation>
  : base_storage<Entity, data_kind::relation> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, data_kind::relation>;

    using base_storage<Entity, data_kind::relation>::store;

    virtual auto store(entity_param subject, entity_param object, Relation&&)
      -> Relation* = 0;

    virtual void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>,
      entity_param subject,
      entity_param object) = 0;

    virtual void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>,
      iterator_t&) = 0;

    virtual void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>,
      entity_param subject,
      entity_param object) = 0;

    virtual void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>,
      iterator_t&) = 0;

    using base_storage<Entity, data_kind::relation>::for_each;

    virtual void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>,
      entity_param subject) = 0;

    virtual void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>,
      entity_param subject) = 0;

    virtual void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>) = 0;

    virtual void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>) = 0;
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

