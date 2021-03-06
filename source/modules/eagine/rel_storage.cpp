/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:rel_storage;

import eagine.core.types;
import eagine.core.reflection;
import eagine.core.utility;
import :entity_traits;
import :manipulator;
import :storage;
import <utility>;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export template <typename Entity>
struct storage_iterator_intf<Entity, true>
  : interface<storage_iterator_intf<Entity, true>> {

    virtual auto reset() -> void = 0;

    virtual auto done() -> bool = 0;

    virtual void next() = 0;

    virtual auto subject() -> Entity = 0;

    virtual auto object() -> Entity = 0;
};
//------------------------------------------------------------------------------
export template <typename Entity>
class storage_iterator<Entity, true> {
public:
    storage_iterator(storage_iterator_intf<Entity, true>* i) noexcept
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

    auto release() -> storage_iterator_intf<Entity, true>* {
        return std::exchange(_i, nullptr);
    }

    auto ptr() noexcept -> storage_iterator_intf<Entity, true>* {
        assert(_i);
        return _i;
    }

    auto get() noexcept -> storage_iterator_intf<Entity, true>& {
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
    storage_iterator_intf<Entity, true>* _i{nullptr};
};
//------------------------------------------------------------------------------
export template <typename Entity>
struct base_storage<Entity, true> : interface<base_storage<Entity, true>> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, true>;

    virtual auto capabilities() -> storage_caps = 0;

    virtual auto new_iterator() -> iterator_t = 0;

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
struct storage<Entity, Relation, true> : base_storage<Entity, true> {
    using entity_param = entity_param_t<Entity>;
    using iterator_t = storage_iterator<Entity, true>;

    using base_storage<Entity, true>::store;

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

    using base_storage<Entity, true>::for_each;

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

