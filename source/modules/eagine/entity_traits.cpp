/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.ecs:entity_traits;

import std;
import eagine.core.identifier;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export enum class data_kind : bool { component = false, relation = true };
//------------------------------------------------------------------------------
export template <typename Entity>
struct entity_traits {
    using parameter_type = const Entity;

    [[nodiscard]] static constexpr auto first() noexcept -> Entity {
        return Entity();
    }

    [[nodiscard]] static constexpr auto next(parameter_type i) noexcept
      -> Entity {
        return ++i;
    }
};

export template <typename Entity>
using entity_param_t = typename entity_traits<Entity>::parameter_type;
//------------------------------------------------------------------------------
export template <>
struct entity_traits<std::string> {
    using parameter_type = const std::string&;

    [[nodiscard]] static constexpr auto first() noexcept -> std::string {
        return {};
    }
};
//------------------------------------------------------------------------------
export template <std::size_t M, std::size_t B, typename C, typename T>
struct entity_traits<basic_identifier<M, B, C, T>> {
    using parameter_type = const basic_identifier<M, B, C, T>;

    [[nodiscard]] static constexpr auto first() noexcept
      -> basic_identifier<M, B, C, T> {
        return {};
    }

    [[nodiscard]] static constexpr auto next(parameter_type i) noexcept
      -> basic_identifier<M, B, C, T> {
        return increment(i);
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

