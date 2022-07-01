/// @example eagine/ecs/001_hello.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#if EAGINE_ECS_MODULE
import eagine.core;
import eagine.ecs;
import <string>;
#else
#include <eagine/console/console.hpp>
#include <eagine/ecs/basic_manager.hpp>
#include <eagine/ecs/component.hpp>
#include <eagine/ecs/manipulator.hpp>
#include <eagine/ecs/storage/std_map.hpp>
#include <eagine/logging/logger.hpp>
#include <eagine/main_ctx.hpp>
#endif

namespace eagine {

struct object : ecs::component<id_v("Object")> {
    std::string name;
};

struct greeting : ecs::component<id_v("Greeting")> {
    std::string expression;
};

auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.log().info("starting");

    ecs::basic_manager<identifier_t> mgr;
    mgr.register_component_storage<ecs::std_map_cmp_storage, object>();
    mgr.register_component_storage<ecs::std_map_cmp_storage, greeting>();

    const auto hw = id_v("HelloWorld");
    const auto he = id_v("HelloNtity");

    mgr.add<greeting>(hw).write().expression = "Hello";
    mgr.add<object>(hw).write().name = "World";

    mgr.copy<greeting>(hw, he);
    mgr.add<object>(he).write().name = "Entity";

    mgr.for_each_with<const greeting, const object>(
      [&](const auto&, auto& grt, auto& obj) {
          ctx.cio()
            .print(identifier{"ECS"}, "${expr}, ${name}")
            .arg(identifier{"expr"}, grt.read().expression)
            .arg(identifier{"name"}, obj.read().name);
      });

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

