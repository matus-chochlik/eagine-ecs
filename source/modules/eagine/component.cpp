/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.ecs:component;

import std;
import eagine.core.types;
import eagine.core.container;
export import eagine.core.identifier;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export enum class data_kind : bool { component = false, relation = true };
//------------------------------------------------------------------------------
// entity_data
export template <identifier_value Uid, data_kind Kind>
struct entity_data {
    [[nodiscard]] static constexpr auto uid() noexcept {
        return Uid;
    }

    [[nodiscard]] static constexpr auto kind() noexcept {
        return Kind;
    }

    [[nodiscard]] static constexpr auto is_component() noexcept {
        return Kind == data_kind::component;
    }

    [[nodiscard]] static constexpr auto is_relation() noexcept {
        return Kind == data_kind::relation;
    }
};
//------------------------------------------------------------------------------
// component - base class
export template <identifier_value Uid>
using component = entity_data<Uid, data_kind::component>;

export template <typename T>
concept component_data = requires(const T& x) {
    { x.is_component() } -> std::convertible_to<bool>;
    bool(x.is_component());
};
//------------------------------------------------------------------------------
// relation - base class
export template <identifier_value Uid>
using relation = entity_data<Uid, data_kind::relation>;

export template <typename T>
concept relation_data = requires(const T& x) {
    { x.is_relation() } -> std::convertible_to<bool>;
    bool(x.is_relation());
};
//------------------------------------------------------------------------------
// component_uid_map
export template <typename T>
class component_uid_map {
public:
    [[nodiscard]] auto find(const identifier_value cid) noexcept {
        return eagine::find(_storage, cid);
    }

    [[nodiscard]] auto find(const identifier_value cid) const noexcept {
        return eagine::find(_storage, cid);
    }

    template <typename... Args>
    auto emplace(const identifier_value cid, Args&&... args) -> bool {
        return _storage.emplace(cid, std::forward<Args>(args)...).second;
    }

    template <typename I, typename S>
    auto erase(const optional_iterator<I, S, T>& pos) {
        return _storage.erase(pos);
    }

    auto erase(const identifier_value cid) {
        return _storage.erase(cid);
    }

    void clear() {
        _storage.clear();
    }

    [[nodiscard]] auto begin() noexcept {
        return _storage.begin();
    }

    [[nodiscard]] auto end() noexcept {
        return _storage.end();
    }

    auto operator[](const identifier_value cid) -> T& {
        return _storage[cid];
    }

private:
    flat_map<identifier_t, T> _storage{};
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

