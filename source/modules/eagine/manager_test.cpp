/// @file
///
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_ctx.hpp>
import eagine.core;
import eagine.ecs;
//------------------------------------------------------------------------------
struct subject : eagine::ecs::component<"Subject"> {
    std::string name;
};

struct greeting : eagine::ecs::component<"Greeting"> {
    std::string expression;
};
//------------------------------------------------------------------------------
void manager_component_write_has_1(auto& s) {
    eagitest::case_ test{s, 1, "write/has"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const auto hw = eagine::id_v("HelloWorld");

    test.check(not mgr.has<greeting>(hw), "has not greeting");
    test.check(not mgr.has<subject>(hw), "has not subject");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    mgr.ensure<subject>(hw).write().name = "World";

    test.check(mgr.has<greeting>(hw), "has greeting");
    test.check(mgr.has<subject>(hw), "has subject");
}
//------------------------------------------------------------------------------
void manager_component_write_get_1(auto& s) {
    eagitest::case_ test{s, 2, "write/get"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const std::string na{"N/A"};
    const auto hw = eagine::id_v("Hello");

    test.check(mgr.get(&greeting::expression, hw, na) == na, "no greeting");
    test.check(mgr.get(&subject::name, hw, na) == na, "no subject name");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    test.check(mgr.get(&greeting::expression, hw, na) == "Hello", "greeting");
    test.check(mgr.get(&subject::name, hw, na) == na, "no subject name");

    mgr.ensure<subject>(hw).write().name = "World";
    test.check(mgr.get(&greeting::expression, hw, na) == "Hello", "greeting");
    test.check(mgr.get(&subject::name, hw, na) == "World", "subject name");
}
//------------------------------------------------------------------------------
void manager_component_write_read_1(auto& s) {
    eagitest::case_ test{s, 3, "write/read"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const auto hw = eagine::id_v("Hello");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    mgr.ensure<subject>(hw).write().name = "World";

    test.check(mgr.ensure<greeting>(hw).read().expression == "Hello", "hello");
    test.check(mgr.ensure<subject>(hw).read().name == "World", "world");
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "manager", 3};
    test.once(manager_component_write_has_1);
    test.once(manager_component_write_get_1);
    test.once(manager_component_write_read_1);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
