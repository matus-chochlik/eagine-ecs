/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_ECS_COMPONENT_HPP
#define EAGINE_ECS_COMPONENT_HPP

#include <eagine/config/basic.hpp>
#include <eagine/flat_map.hpp>
#include <eagine/identifier_t.hpp>
#include <eagine/optional_ref.hpp>
#include <type_traits>

namespace eagine::ecs {

// component unique identifier
using component_uid_t = identifier_t;

// entity_data
template <typename Derived, component_uid_t Uid, bool IsRelation>
struct entity_data {
    static constexpr auto uid() noexcept {
        return Uid;
    }

    static constexpr auto is_relation() noexcept {
        return IsRelation;
    }
};

// component - base class
template <typename Derived, component_uid_t Uid>
using component = entity_data<Derived, Uid, false>;

// relation - base class
template <typename Derived, component_uid_t Uid>
using relation = entity_data<Derived, Uid, true>;

// component_uid_map
template <typename T>
class component_uid_map {
public:
    auto find(component_uid_t cid) noexcept -> optional_reference_wrapper<T> {
        const auto pos{_storage.find(cid)};
        if(pos != _storage.end()) {
            return {pos->second};
        }
        return {nothing};
    }

    auto find(component_uid_t cid) const noexcept
      -> optional_reference_wrapper<const T> {
        const auto pos{_storage.find(cid)};
        if(pos != _storage.end()) {
            return {pos->second};
        }
        return {nothing};
    }

    template <typename... Args>
    auto emplace(component_uid_t cid, Args&&... args) -> bool {
        return _storage.emplace(cid, std::forward<Args>(args)...).second;
    }

    auto erase(component_uid_t cid) {
        return _storage.erase(cid);
    }

    auto operator[](component_uid_t cid) -> T& {
        return _storage[cid];
    }

private:
    flat_map<component_uid_t, T> _storage{};
};

} // namespace eagine::ecs

#endif // EAGINE_ECS_COMPONENT_HPP
