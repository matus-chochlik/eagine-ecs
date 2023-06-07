/// @example eagine/ecs/001_hello_manager.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.ecs;
import std;

namespace eagine {

struct subject : ecs::component<"Subject"> {
    std::string name;
};

struct greeting : ecs::component<"Greeting"> {
    std::string expression;
};

auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.log().info("starting");

    ecs::basic_manager<identifier_t> mgr;
    mgr.register_component_storage<ecs::std_map_cmp_storage, subject>();
    mgr.register_component_storage<ecs::std_map_cmp_storage, greeting>();

    const auto hw = id_v("HelloWorld");
    const auto he = id_v("HelloNtity");

    mgr.ensure<greeting>(hw)->expression = "Hello";
    mgr.ensure<subject>(hw)->name = "World";

    mgr.copy<greeting>(hw, he);
    mgr.ensure<subject>(he)->name = "Entity";

    mgr.for_each_with<const greeting, const subject>(
      [&](const auto&, auto& grt, auto& sub) {
          ctx.cio()
            .print("ECS", "${expr}, ${name}")
            .arg("expr", grt->expression)
            .arg("name", sub->name);
      });

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

