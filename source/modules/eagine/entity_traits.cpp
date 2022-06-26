/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.ecs:entity_traits;

import <string>;

namespace eagine::ecs {

export template <typename Entity>
struct entity_traits {
    using parameter_type = const Entity;

    static auto minimum() noexcept -> Entity {
        return Entity();
    }
};

export template <typename Entity>
using entity_param_t = typename entity_traits<Entity>::parameter_type;

export template <>
struct entity_traits<std::string> {
    using parameter_type = const std::string&;

    static inline auto minimum() noexcept -> std::string {
        return {};
    }
};

} // namespace eagine::ecs

