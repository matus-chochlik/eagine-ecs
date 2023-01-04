/// @file
///
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_ctx.hpp>
import eagine.core;
import eagine.ecs;
import <map>;
import <tuple>;
//------------------------------------------------------------------------------
struct subject : eagine::ecs::component<"Subject"> {
    subject() noexcept = default;
    subject(std::string n, std::string fn) noexcept
      : name{std::move(n)}
      , family_name{std::move(fn)} {}

    std::string name;
    std::string family_name;
};

template <bool Const>
struct subject_manipulator : eagine::ecs::basic_manipulator<subject, Const> {
    using eagine::ecs::basic_manipulator<subject, Const>::basic_manipulator;

    auto set(std::string name, std::string family_name) -> auto& {
        this->write().name.assign(std::move(name));
        this->write().family_name.assign(std::move(family_name));
        return *this;
    }

    auto has_name(std::string_view name, std::string_view family_name) noexcept
      -> bool {
        return (this->read().name == name) and
               (this->read().family_name == family_name);
    }
};
//------------------------------------------------------------------------------
struct greeting : eagine::ecs::component<"Greeting"> {
    greeting() noexcept = default;
    greeting(std::string e) noexcept
      : expression{std::move(e)} {}

    std::string expression;
};
//------------------------------------------------------------------------------
namespace eagine::ecs {
template <bool Const>
struct get_manipulator<::subject, Const> {
    using type = ::subject_manipulator<Const>;
};
} // namespace eagine::ecs
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
void manager_component_manipulator_1(auto& s) {
    eagitest::case_ test{s, 4, "manipulator"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();

    const std::string na{"N/A"};
    const auto johnny = eagine::id_v("Johnny");

    test.check_equal(mgr.get(&subject::name, johnny, na), na, "no name");
    test.check_equal(
      mgr.get(&subject::family_name, johnny, na), na, "no family name");

    mgr.ensure<subject>(johnny).set("John", "Doe");

    test.check_equal(mgr.get(&subject::name, johnny, na), "John", "name");
    test.check_equal(
      mgr.get(&subject::family_name, johnny, na), "Doe", "family name");

    test.check(
      mgr.ensure<subject>(johnny).has_name("John", "Doe"), "has name 1");
    test.check(
      not mgr.ensure<subject>(johnny).has_name("Jane", "Doe"), "has name 2");
    test.check(
      not mgr.ensure<subject>(johnny).has_name("John", "Roe"), "has name 3");
    test.check(
      not mgr.ensure<subject>(johnny).has_name("Bill", "Roe"), "has name 4");
}
//------------------------------------------------------------------------------
void manager_component_add_has_name_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 5, "add/get"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    mgr.add(id_v("john"), subject("John", "Doe"));
    mgr.add(id_v("john"), greeting("Hi"));
    mgr.add(id_v("jane"), subject("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("bill"), subject("Bill", "Roe"))
      .add(id_v("bill"), greeting("Hello"));

    test.check(
      mgr.ensure<subject>(id_v("john")).has_name("John", "Doe"), "has name 1");
    test.check(
      mgr.ensure<subject>(id_v("jane")).has_name("Jane", "Doe"), "has name 2");
    test.check(
      mgr.ensure<subject>(id_v("bill")).has_name("Bill", "Roe"), "has name 3");
}
//------------------------------------------------------------------------------
void manager_component_for_each_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 6, "for-each"};
    eagitest::track trck{test, 0, 3};

    std::map<eagine::identifier_t, std::tuple<std::string, std::string>> names;

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, subject>();

    const auto add = [&](auto eid, std::string name, std::string family_name) {
        names[eid] = {name, family_name};
        mgr.add(eid, subject(std::move(name), std::move(family_name)));
    };

    add(id_v("john"), "John", "Roe");
    add(id_v("jane"), "Jane", "Roe");
    add(id_v("bill"), "Bill", "Doe");
    add(id_v("jack"), "Jack", "Daniels");

    mgr.read_each<subject>([&](auto eid, auto& sub) {
        const auto& [name, family_name] = names[eid];
        test.check(sub.has_name(name, family_name), "name");
        trck.checkpoint(1);
    });

    mgr.write_each<subject>([&](auto, auto& sub) {
        sub->family_name = "X";
        trck.checkpoint(2);
    });

    mgr.read_each<subject>([&](auto eid, auto& sub) {
        const auto& name = std::get<0>(names[eid]);
        test.check(sub.has_name(name, "X"), "name");
        trck.checkpoint(3);
    });
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "manager", 6};
    test.once(manager_component_write_has_1);
    test.once(manager_component_write_get_1);
    test.once(manager_component_write_read_1);
    test.once(manager_component_manipulator_1);
    test.once(manager_component_add_has_name_1);
    test.once(manager_component_for_each_1);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
