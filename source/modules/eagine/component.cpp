/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.ecs:component;

import eagine.core.types;
export import eagine.core.container;
import <type_traits>;

namespace eagine::ecs {

// component unique identifier
export using component_uid_t = identifier_t;

export enum class data_kind : bool { component = false, relation = true };

// entity_data
export template <component_uid_t Uid, data_kind Kind>
struct entity_data {
    static constexpr auto uid() noexcept {
        return Uid;
    }

    static constexpr auto kind() noexcept {
        return Kind;
    }

    static constexpr auto is_component() noexcept {
        return Kind == data_kind::component;
    }

    static constexpr auto is_relation() noexcept {
        return Kind == data_kind::relation;
    }
};

// component - base class
export template <component_uid_t Uid>
using component = entity_data<Uid, data_kind::component>;

// relation - base class
export template <component_uid_t Uid>
using relation = entity_data<Uid, data_kind::relation>;

// component_uid_map
export template <typename T>
class component_uid_map {
public:
    auto find(const component_uid_t cid) noexcept
      -> optional_reference_wrapper<T> {
        const auto pos{_storage.find(cid)};
        if(pos != _storage.end()) {
            return {pos->second};
        }
        return {nothing};
    }

    auto find(const component_uid_t cid) const noexcept
      -> optional_reference_wrapper<const T> {
        const auto pos{_storage.find(cid)};
        if(pos != _storage.end()) {
            return {pos->second};
        }
        return {nothing};
    }

    template <typename... Args>
    auto emplace(const component_uid_t cid, Args&&... args) -> bool {
        return _storage.emplace(cid, std::forward<Args>(args)...).second;
    }

    auto erase(const component_uid_t cid) {
        return _storage.erase(cid);
    }

    auto operator[](const component_uid_t cid) -> T& {
        return _storage[cid];
    }

private:
    flat_map<component_uid_t, T> _storage{};
};

} // namespace eagine::ecs

