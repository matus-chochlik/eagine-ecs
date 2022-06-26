/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.ecs:storage;

import eagine.core.types;
import eagine.core.reflection;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export enum class storage_cap_bit : unsigned short {
    hide = 1U << 0U,
    copy = 1U << 1U,
    swap = 1U << 2U,
    store = 1U << 3U,
    remove = 1U << 4U,
    modify = 1U << 5U
};
//------------------------------------------------------------------------------
export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<storage_cap_bit>,
  const Selector) noexcept {
    return enumerator_map_type<storage_cap_bit, 6>{
      {{"hide", storage_cap_bit::hide},
       {"copy", storage_cap_bit::copy},
       {"swap", storage_cap_bit::swap},
       {"store", storage_cap_bit::store},
       {"remove", storage_cap_bit::remove},
       {"modify", storage_cap_bit::modify}}};
}
//------------------------------------------------------------------------------
export auto operator|(const storage_cap_bit a, const storage_cap_bit b) noexcept
  -> bitfield<storage_cap_bit> {
    return {a, b};
}
//------------------------------------------------------------------------------
export class storage_caps : public bitfield<storage_cap_bit> {
    using _base = bitfield<storage_cap_bit>;

public:
    storage_caps() noexcept = default;

    storage_caps(const bitfield<storage_cap_bit> base)
      : _base{base} {}

    auto can_hide() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    auto can_copy() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    auto can_swap() const noexcept -> bool {
        return has(storage_cap_bit::hide);
    }

    auto can_remove() const noexcept -> bool {
        return has(storage_cap_bit::remove);
    }

    auto can_store() const noexcept -> bool {
        return has(storage_cap_bit::store);
    }

    auto can_modify() const noexcept -> bool {
        return has(storage_cap_bit::modify);
    }
};
//------------------------------------------------------------------------------
template <typename Entity, bool IsRelation>
struct storage_iterator_intf;

template <typename Entity>
using component_storage_iterator_intf = storage_iterator_intf<Entity, false>;

template <typename Entity>
using relation_storage_iterator_intf = storage_iterator_intf<Entity, true>;

template <typename Entity, bool IsRelation>
class storage_iterator;

template <typename Entity>
using component_storage_iterator = storage_iterator<Entity, false>;

template <typename Entity>
using relation_storage_iterator = storage_iterator<Entity, true>;

template <typename Entity, bool IsRelation>
struct base_storage;

template <typename Entity>
using base_component_storage = base_storage<Entity, false>;

template <typename Entity>
using base_relation_storage = base_storage<Entity, true>;

template <typename Entity, typename Data, bool IsRelation>
struct storage;

template <typename Entity, typename Data>
using component_storage = storage<Entity, Data, false>;

template <typename Entity, typename Data>
using relation_storage = storage<Entity, Data, true>;
//------------------------------------------------------------------------------
} // namespace eagine::ecs

