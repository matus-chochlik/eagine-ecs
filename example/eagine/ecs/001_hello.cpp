/// @example eagine/ecs/001_hello.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.ecs;
import <string>;

namespace eagine {

struct object : ecs::component<"Object"> {
    std::string name;
};

struct greeting : ecs::component<"Greeting"> {
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

