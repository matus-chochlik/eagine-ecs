/// @example eagine/ecs/002_signals.hpp
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
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.log().info("starting");

    auto& mgr = enable_ecs(ctx).value();

    mgr.register_component_storages<eagine::ecs::std_map_cmp_storage, subject>();

    const auto on_spawned{[&](identifier e) {
        ctx.cio().print("ECS", "Spawned ${name}").arg("name", e);
    }};
    mgr.entity_spawned.connect({construct_from, on_spawned});

    const auto on_forgotten{[&](identifier e) {
        ctx.cio().print("ECS", "Forgotten ${name}").arg("name", e);
    }};
    mgr.entity_forgotten.connect({construct_from, on_forgotten});

    std::vector<ecs::object> objects;

    for(int i = 0; i < 64; ++i) {
        auto obj{ecs::object::spawn(ctx)};
        obj.ensure<subject>()->name = obj.entity().name().str();
        objects.emplace_back(std::move(obj));
    }

    objects.clear();

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

