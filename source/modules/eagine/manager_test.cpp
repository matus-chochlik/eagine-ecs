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
struct person : eagine::ecs::component<"Person"> {
    person() noexcept = default;
    person(std::string n, std::string fn) noexcept
      : name{std::move(n)}
      , family_name{std::move(fn)} {}

    std::string name;
    std::string family_name;
};

template <bool Const>
struct person_manipulator : eagine::ecs::basic_manipulator<person, Const> {
    using eagine::ecs::basic_manipulator<person, Const>::basic_manipulator;

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
struct father : eagine::ecs::relation<"Father"> {};
struct mother : eagine::ecs::relation<"Mother"> {};
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
struct get_manipulator<::person, Const> {
    using type = ::person_manipulator<Const>;
};
} // namespace eagine::ecs
//------------------------------------------------------------------------------
// register / unregister
//------------------------------------------------------------------------------
void manager_component_register_1(auto& s) {
    eagitest::case_ test{s, 1, "register component"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;

    test.check(not mgr.knows_component_type<person>(), "person 1");
    test.check(not mgr.knows_component_type<greeting>(), "greeting 1");
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();

    test.check(mgr.knows_component_type<person>(), "person 2");
    test.check(not mgr.knows_component_type<greeting>(), "greeting 2");
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    test.check(mgr.knows_component_type<person>(), "person 3");
    test.check(mgr.knows_component_type<greeting>(), "greeting 3");

    mgr.unregister_component_type<person>();
    test.check(not mgr.knows_component_type<person>(), "person 4");
    test.check(mgr.knows_component_type<greeting>(), "greeting 4");

    mgr.unregister_component_type<greeting>();
    test.check(not mgr.knows_component_type<person>(), "person 5");
    test.check(not mgr.knows_component_type<greeting>(), "greeting 5");
}
//------------------------------------------------------------------------------
// register / unregister
//------------------------------------------------------------------------------
void manager_component_register_2(auto& s) {
    eagitest::case_ test{s, 2, "register relation"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;

    test.check(not mgr.knows_relation_type<mother>(), "mother 1");
    test.check(not mgr.knows_relation_type<father>(), "father 1");
    mgr.register_relation_storage<eagine::ecs::std_map_rel_storage, mother>();

    test.check(mgr.knows_relation_type<mother>(), "mother 2");
    test.check(not mgr.knows_relation_type<father>(), "father 2");
    mgr.register_relation_storage<eagine::ecs::std_map_rel_storage, father>();

    test.check(mgr.knows_relation_type<mother>(), "mother 3");
    test.check(mgr.knows_relation_type<father>(), "father 3");

    mgr.unregister_relation_type<mother>();
    test.check(not mgr.knows_relation_type<mother>(), "mother 4");
    test.check(mgr.knows_relation_type<father>(), "father 4");

    mgr.unregister_relation_type<father>();
    test.check(not mgr.knows_relation_type<mother>(), "mother 5");
    test.check(not mgr.knows_relation_type<father>(), "father 5");
}
//------------------------------------------------------------------------------
// write / has
//------------------------------------------------------------------------------
void manager_component_write_has_1(auto& s) {
    eagitest::case_ test{s, 3, "write/has"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const auto hw = eagine::id_v("HelloWorld");

    test.check(not mgr.has<greeting>(hw), "has not greeting");
    test.check(not mgr.has<person>(hw), "has not person");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    mgr.ensure<person>(hw).write().name = "World";

    test.check(mgr.has<greeting>(hw), "has greeting");
    test.check(mgr.has<person>(hw), "has person");
}
//------------------------------------------------------------------------------
// write / get
//------------------------------------------------------------------------------
void manager_component_write_get_1(auto& s) {
    eagitest::case_ test{s, 4, "write/get"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const std::string na{"N/A"};
    const auto hw = eagine::id_v("Hello");

    test.check(mgr.get(&greeting::expression, hw, na) == na, "no greeting");
    test.check(mgr.get(&person::name, hw, na) == na, "no person name");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    test.check(mgr.get(&greeting::expression, hw, na) == "Hello", "greeting");
    test.check(mgr.get(&person::name, hw, na) == na, "no person name");

    mgr.ensure<person>(hw).write().name = "World";
    test.check(mgr.get(&greeting::expression, hw, na) == "Hello", "greeting");
    test.check(mgr.get(&person::name, hw, na) == "World", "person name");
}
//------------------------------------------------------------------------------
// write / read
//------------------------------------------------------------------------------
void manager_component_write_read_1(auto& s) {
    eagitest::case_ test{s, 5, "write/read"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    const auto hw = eagine::id_v("Hello");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    mgr.ensure<person>(hw).write().name = "World";

    test.check(mgr.ensure<greeting>(hw).read().expression == "Hello", "hello");
    test.check(mgr.ensure<person>(hw).read().name == "World", "world");
}
//------------------------------------------------------------------------------
// manipulator
//------------------------------------------------------------------------------
void manager_component_manipulator_1(auto& s) {
    eagitest::case_ test{s, 6, "manipulator"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();

    const std::string na{"N/A"};
    const auto johnny = eagine::id_v("Johnny");

    test.check_equal(mgr.get(&person::name, johnny, na), na, "no name");
    test.check_equal(
      mgr.get(&person::family_name, johnny, na), na, "no family name");

    mgr.ensure<person>(johnny).set("John", "Doe");

    test.check_equal(mgr.get(&person::name, johnny, na), "John", "name");
    test.check_equal(
      mgr.get(&person::family_name, johnny, na), "Doe", "family name");

    test.check(
      mgr.ensure<person>(johnny).has_name("John", "Doe"), "has name 1");
    test.check(
      not mgr.ensure<person>(johnny).has_name("Jane", "Doe"), "has name 2");
    test.check(
      not mgr.ensure<person>(johnny).has_name("John", "Roe"), "has name 3");
    test.check(
      not mgr.ensure<person>(johnny).has_name("Bill", "Roe"), "has name 4");
}
//------------------------------------------------------------------------------
// add / has_name
//------------------------------------------------------------------------------
void manager_component_add_has_name_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 7, "add/get"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    mgr.add(id_v("john"), person("John", "Doe"));
    mgr.add(id_v("john"), greeting("Hi"));
    mgr.add(id_v("jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("bill"), person("Bill", "Roe"))
      .add(id_v("bill"), greeting("Hello"));

    test.check(
      mgr.ensure<person>(id_v("john")).has_name("John", "Doe"), "has name 1");
    test.check(
      mgr.ensure<person>(id_v("jane")).has_name("Jane", "Doe"), "has name 2");
    test.check(
      mgr.ensure<person>(id_v("bill")).has_name("Bill", "Roe"), "has name 3");
}
//------------------------------------------------------------------------------
// for-single
//------------------------------------------------------------------------------
void manager_component_for_single_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 8, "for-single"};

    eagine::ecs::basic_manager<std::string> mgr;

    mgr.ensure<person>("john");
    mgr.ensure<person>("jane");

    mgr.write_single<person>("john", [&](const auto& ent, auto& p) {
        test.check(ent == "john", "john id");
        p.set("John", "Doe");
    });

    mgr.write_single<person>("jane", [&](const auto& ent, auto& p) {
        test.check(ent == "jane", "jane id");
        p.set("Jane", "Roe");
    });

    mgr.read_single<person>("jane", [&](const auto& ent, auto& p) {
        test.check(ent == "jane", "jane id");
        test.check(p.has_name("Jane", "Roe"), "has name jane");
    });

    mgr.read_single<person>("john", [&](const auto& ent, auto& p) {
        test.check(ent == "john", "john id");
        test.check(p.has_name("John", "Doe"), "has name john");
    });
}
//------------------------------------------------------------------------------
// for-each
//------------------------------------------------------------------------------
void manager_component_for_each_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 9, "for-each"};
    eagitest::track trck{test, 0, 3};

    std::map<eagine::identifier_t, std::tuple<std::string, std::string>> names;

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();

    const auto add = [&](auto eid, std::string name, std::string family_name) {
        names[eid] = {name, family_name};
        mgr.add(eid, person(std::move(name), std::move(family_name)));
    };

    add(id_v("john"), "John", "Roe");
    add(id_v("jane"), "Jane", "Roe");
    add(id_v("bill"), "Bill", "Doe");
    add(id_v("jack"), "Jack", "Daniels");

    mgr.read_each<person>([&](auto eid, auto& sub) {
        const auto& [name, family_name] = names[eid];
        test.check(sub.has_name(name, family_name), "name");
        trck.checkpoint(1);
    });

    mgr.write_each<person>([&](auto, auto& sub) {
        sub->family_name = "X";
        trck.checkpoint(2);
    });

    mgr.read_each<person>([&](auto eid, auto& sub) {
        const auto& name = std::get<0>(names[eid]);
        test.check(sub.has_name(name, "X"), "name");
        trck.checkpoint(3);
    });
}
//------------------------------------------------------------------------------
// has / has-all
//------------------------------------------------------------------------------
void manager_component_has_1(auto& s) {
    eagitest::case_ test{s, 10, "has"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    mgr.add("john", person("John", "Doe"));
    mgr.add("jane", person("Jane", "Doe"), greeting("Hi"));
    mgr.add("bill", person("Bill", "Roe")).add("bill", greeting("Hello"));
    mgr.add("unknown", greeting("Howdy"));

    test.check(not mgr.has<person>("missing"), "has not (missing)");
    test.check(mgr.has<person>("john"), "has (john)");
    test.check(mgr.has<person>("jane"), "has (jane)");
    test.check(mgr.has<person>("bill"), "has (bill)");
    test.check(not mgr.has<greeting>("john"), "has not (john)");
    test.check(mgr.has<greeting>("jane"), "has (jane)");
    test.check(mgr.has<greeting>("bill"), "has (bill)");

    test.check(
      not mgr.has_all<person, greeting>("missing"), "has not all (missing)");
    test.check(
      not mgr.has_all<person, greeting>("unknown"), "has not all (unknown)");
    test.check(not mgr.has_all<person, greeting>("john"), "has not all (john)");
    test.check(mgr.has_all<person, greeting>("jane"), "has all (jane)");
    test.check(mgr.has_all<person, greeting>("bill"), "has all (bill)");
}
//------------------------------------------------------------------------------
// show / hide
//------------------------------------------------------------------------------
void manager_component_show_hide_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 11, "show/hide"};
    eagitest::track trck{test, 0, 2};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, greeting>();

    mgr.add(id_v("john"), person("John", "Doe"), greeting("Hi"));
    mgr.add(id_v("jane"), person("Jane", "Doe"), greeting("Hello"));
    mgr.add(id_v("bill"), person("Bill", "Roe"));

    test.check(not mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");

    test.check(
      not mgr.are_hidden<person, greeting>(id_v("john")),
      "are not hidden (john)");
    test.check(
      not mgr.are_hidden<greeting, person>(id_v("jane")),
      "are not hidden (jane)");
    test.check(
      not mgr.are_hidden<person>(id_v("bill")), "are not hidden (bill)");

    mgr.hide<person>(id_v("john"));

    test.check(mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");

    mgr.hide<greeting>(id_v("jane"));

    test.check(mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");

    mgr.hide<person>(id_v("jane"));

    test.check(mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");
    test.check(
      mgr.are_hidden<greeting, person>(id_v("jane")), "are not hidden (jane)");

    mgr.hide<greeting>(id_v("john"));

    test.check(mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");
    test.check(
      mgr.are_hidden<greeting, person>(id_v("john")), "are not hidden (john)");
    test.check(
      mgr.are_hidden<greeting, person>(id_v("jane")), "are not hidden (jane)");

    mgr.read_each<person>([&](auto eid, auto&) {
        test.check(eid == id_v("bill"), "only bill");
        trck.checkpoint(1);
    });

    mgr.show<greeting, person>(id_v("jane"));

    test.check(mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");
    test.check(
      mgr.are_hidden<greeting, person>(id_v("john")), "are not hidden (john)");
    test.check(
      not mgr.are_hidden<greeting, person>(id_v("jane")),
      "are not hidden (jane)");

    mgr.read_each<person>([&](auto eid, auto&) {
        test.check(eid == id_v("bill") or eid == id_v("jane"), "bill or jane");
        trck.checkpoint(2);
    });

    mgr.show<person, greeting>(id_v("john"));

    test.check(not mgr.is_hidden<person>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<greeting>(id_v("john")), "not hidden (john)");
    test.check(not mgr.is_hidden<person>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<greeting>(id_v("jane")), "not hidden (jane)");
    test.check(not mgr.is_hidden<person>(id_v("bill")), "not hidden (bill)");
    test.check(not mgr.is_hidden<greeting>(id_v("bill")), "not hidden (bill)");
    test.check(
      not mgr.are_hidden<greeting, person>(id_v("john")),
      "are not hidden (john)");
    test.check(
      not mgr.are_hidden<person, greeting>(id_v("jane")),
      "are not hidden (jane)");
}
//------------------------------------------------------------------------------
// relation / has
//------------------------------------------------------------------------------
void manager_component_relation_has_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 12, "relation/has"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::std_map_cmp_storage, person>();
    mgr.register_relation_storage<eagine::ecs::std_map_rel_storage, father>();
    mgr.register_relation_storage<eagine::ecs::std_map_rel_storage, mother>();

    mgr.ensure<person>("force").set("The", "Force");
    mgr.ensure<person>("luke").set("Luke", "Skywalker");
    mgr.ensure<person>("leia").set("Leia", "Organa");
    mgr.ensure<person>("hans").set("Hans", "Olo");
    mgr.ensure<person>("vader").set("Anakin", "Skywalker");
    mgr.ensure<person>("padme").set("Padme", "Amidala");
    mgr.ensure<person>("shmi").set("Shmi", "Skywalker");
    mgr.ensure<person>("jarjar").set("Jar-Jar", "Binks");
    mgr.ensure<person>("chewie").set("Chewbacca", "N/A");
    mgr.ensure<person>("yoda").set("Yoda", "N/A");
    mgr.ensure<person>("angryguy").set("Kylo", "Ren");

    mgr.ensure<father>("luke", "vader");
    mgr.ensure<father>("leia", "vader");
    mgr.ensure<mother>("luke", "padme");
    mgr.ensure<mother>("leia", "padme");
    mgr.ensure<mother>("vader", "shmi");
    mgr.ensure<father>("vader", "force");
    mgr.ensure<mother>("angryguy", "leia");
    mgr.ensure<father>("angryguy", "hans");

    test.check(mgr.has<father>("luke", "vader"), "1");
    test.check(mgr.has<father>("leia", "vader"), "2");
    test.check(mgr.has<mother>("luke", "padme"), "3");
    test.check(mgr.has<mother>("leia", "padme"), "4");
    test.check(mgr.has<mother>("vader", "shmi"), "5");
    test.check(mgr.has<father>("vader", "force"), "6");
    test.check(mgr.has<mother>("angryguy", "leia"), "7");
    test.check(mgr.has<father>("angryguy", "hans"), "8");

    mgr.for_each_having<father>(
      {eagine::construct_from, [&](const std::string& c, const std::string& f) {
           test.check(not c.empty(), "not empty");
           test.check(f != "jarjar", "not jarjar");
       }});

    eagitest::track children_of_vader{test, "vader", 2, 1};
    mgr.for_each_having<father>(
      {eagine::construct_from, [&](const std::string& c, const std::string& f) {
           test.check(not c.empty(), "c not empty");
           test.check(not f.empty(), "f not empty");
           if(f == "vader") {
               children_of_vader.checkpoint(1);
           }
       }});

    eagitest::track children_of_leia{test, "leia", 1, 1};
    mgr.for_each_having<mother>(
      {eagine::construct_from, [&](const std::string& c, const std::string& m) {
           test.check(not c.empty(), "c not empty");
           test.check(not m.empty(), "m not empty");
           if(m == "leia") {
               children_of_leia.checkpoint(1);
           }
       }});
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "manager", 12};
    test.once(manager_component_register_1);
    test.once(manager_component_register_2);
    test.once(manager_component_write_has_1);
    test.once(manager_component_write_get_1);
    test.once(manager_component_write_read_1);
    test.once(manager_component_manipulator_1);
    test.once(manager_component_add_has_name_1);
    test.once(manager_component_for_single_1);
    test.once(manager_component_for_each_1);
    test.once(manager_component_has_1);
    test.once(manager_component_show_hide_1);
    test.once(manager_component_relation_has_1);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
