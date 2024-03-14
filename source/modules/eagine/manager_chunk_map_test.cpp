/// @file
///
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_ctx.hpp>
import std;
import eagine.core;
import eagine.ecs;
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
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

    test.check(mgr.knows_component_type<person>(), "person 2");
    test.check(not mgr.knows_component_type<greeting>(), "greeting 2");
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, mother>();

    test.check(mgr.knows_relation_type<mother>(), "mother 2");
    test.check(not mgr.knows_relation_type<father>(), "father 2");
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, father>();

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
// write / knows + has
//------------------------------------------------------------------------------
void manager_component_write_has_1(auto& s) {
    eagitest::case_ test{s, 3, "write/knows+has"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    const auto hw = eagine::id_v("HelloWorld");

    test.check(not mgr.has<greeting>(hw), "has not greeting");
    test.check(not mgr.has<person>(hw), "has not person");
    test.check(not mgr.knows(hw), "has not entity");

    mgr.ensure<greeting>(hw).write().expression = "Hello";
    mgr.ensure<person>(hw).write().name = "World";

    test.check(mgr.has<greeting>(hw), "has greeting");
    test.check(mgr.has<person>(hw), "has person");
    test.check(mgr.knows(hw), "has entity");
}
//------------------------------------------------------------------------------
// write / get
//------------------------------------------------------------------------------
void manager_component_write_get_1(auto& s) {
    eagitest::case_ test{s, 4, "write/get"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

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
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
// add / remove
//------------------------------------------------------------------------------
void manager_component_add_remove_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 8, "add/remove"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    mgr.add(id_v("john"), person("John", "Doe"));
    mgr.add(id_v("jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("bill"), person("Bill", "Roe"))
      .add(id_v("bill"), greeting("Hello"));

    test.check(mgr.has<person>(id_v("john")), "john person");
    test.check(mgr.has<person>(id_v("jane")), "jane person");
    test.check(mgr.has<greeting>(id_v("jane")), "jane greeting");
    test.check(mgr.has<person>(id_v("bill")), "bill person");
    test.check(mgr.has<greeting>(id_v("bill")), "bill greeting");

    mgr.remove<person>(id_v("john"));
    test.check(not mgr.has<person>(id_v("john")), "john not person");

    mgr.add(id_v("john"), greeting("Hi"));
    test.check(mgr.has<greeting>(id_v("john")), "greeting john");

    mgr.remove<greeting>(id_v("jane"));
    test.check(not mgr.has<greeting>(id_v("jane")), "jane not greeting");

    mgr.remove<greeting, person>(id_v("bill"));
    test.check(not mgr.has<person>(id_v("bill")), "bill not person");
    test.check(not mgr.has<greeting>(id_v("bill")), "bill not greeting");

    test.check(mgr.has<greeting>(id_v("john")), "greeting john");
    mgr.remove<person, greeting>(id_v("john"));
    test.check(not mgr.has<person>(id_v("john")), "john not person");
    test.check(not mgr.has<greeting>(id_v("john")), "john not greeting");
}
//------------------------------------------------------------------------------
// add / forget
//------------------------------------------------------------------------------
void manager_component_add_forget_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 9, "add/forget"};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    mgr.add(id_v("john"), greeting("Hi"));
    mgr.add(id_v("john"), person("John", "Doe"));
    mgr.add(id_v("jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("bill"), person("Bill", "Roe"))
      .add(id_v("bill"), greeting("Hello"));

    test.check(mgr.has<person>(id_v("john")), "john person");
    test.check(mgr.has<greeting>(id_v("john")), "greeting john");
    test.check(mgr.has<person>(id_v("jane")), "jane person");
    test.check(mgr.has<greeting>(id_v("jane")), "jane greeting");
    test.check(mgr.has<person>(id_v("bill")), "bill person");
    test.check(mgr.has<greeting>(id_v("bill")), "bill greeting");

    mgr.forget(id_v("john"));
    test.check(not mgr.has<person>(id_v("john")), "john not person");
    test.check(not mgr.has<greeting>(id_v("john")), "john not greeting");

    mgr.forget(id_v("jane"));
    test.check(not mgr.has<person>(id_v("jane")), "jane not person");
    test.check(not mgr.has<greeting>(id_v("jane")), "jane not greeting");

    mgr.forget(id_v("bill"));
    test.check(not mgr.has<person>(id_v("bill")), "bill not person");
    test.check(not mgr.has<greeting>(id_v("bill")), "bill not greeting");
}
//------------------------------------------------------------------------------
// add / copy
//------------------------------------------------------------------------------
void manager_component_add_copy_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 10, "add/copy"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    mgr.add("john1", person("John", "Doe"));

    test.check(not mgr.has<person>("john2"), "john2 not person");
    test.check(not mgr.has<greeting>("john2"), "john2 not greeting");

    mgr.copy<person>("john1", "john2");
    test.check(mgr.has<person>("john2"), "john2 person");
    test.check(
      mgr.ensure<person>("john2").has_name("John", "Doe"), "john2 name");
    test.check(not mgr.has<greeting>("john2"), "john2 not greeting");

    mgr.copy<greeting>("john1", "john2");
    test.check(not mgr.has<greeting>("john2"), "john2 not greeting");

    test.check(not mgr.has<person>("john3"), "john3 not person");
    test.check(not mgr.has<greeting>("john3"), "john3 not greeting");

    mgr.copy<person, greeting>("john2", "john3");
    test.check(mgr.has<person>("john3"), "john3 person");
    test.check(
      mgr.ensure<person>("john3").has_name("John", "Doe"), "john3 name");
    test.check(not mgr.has<greeting>("john3"), "john3 not greeting");

    mgr.add("john1", greeting("Hi"));

    mgr.copy<person>("john1", "john2");
    test.check(mgr.has<person>("john2"), "john2 person");
    test.check(
      mgr.ensure<person>("john2").has_name("John", "Doe"), "john2 name");
    test.check(not mgr.has<greeting>("john2"), "john2 not greeting");

    mgr.copy<greeting>("john1", "john2");
    test.check(mgr.has<greeting>("john2"), "john2 greeting");

    test.check(mgr.has<person>("john3"), "john3 person");
    test.check(not mgr.has<greeting>("john3"), "john3 not greeting");

    mgr.copy<greeting, person>("john2", "john3");
    test.check(mgr.has<person>("john3"), "john3 person");
    test.check(
      mgr.ensure<person>("john3").has_name("John", "Doe"), "john3 name");
    test.check(mgr.has<greeting>("john3"), "john3 greeting");
}
//------------------------------------------------------------------------------
// add / exchange 1
//------------------------------------------------------------------------------
void manager_component_add_exchange_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 11, "add/exchange 1"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    mgr.add("john1", person("John", "Doe"), greeting("Hi"));
    mgr.add("john2", person("John", "Roe"), greeting("Hey"));

    const std::string na{"N/A"};

    test.check(
      mgr.get(&greeting::expression, "john1", na) == "Hi", "john1 greeting 1");
    test.check(
      mgr.get(&greeting::expression, "john2", na) == "Hey", "john2 greeting 1");

    mgr.exchange<greeting>("john1", "john2");

    test.check(
      mgr.get(&greeting::expression, "john1", na) == "Hey", "john1 greeting 2");
    test.check(
      mgr.get(&greeting::expression, "john2", na) == "Hi", "john2 greeting 2");

    test.check(
      mgr.get(&person::family_name, "john1", na) == "Doe", "john1 name 1");
    test.check(
      mgr.get(&person::family_name, "john2", na) == "Roe", "john2 name 1");

    mgr.exchange<person>("john1", "john2");

    test.check(
      mgr.get(&person::family_name, "john1", na) == "Roe", "john1 name 2");
    test.check(
      mgr.get(&person::family_name, "john2", na) == "Doe", "john2 name 2");
}
//------------------------------------------------------------------------------
// add / exchange 2
//------------------------------------------------------------------------------
void manager_component_add_exchange_2(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 12, "add/exchange 2"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

    mgr.add("john1", greeting("Hi"));
    mgr.add("john2", person("John", "Roe"));

    const std::string na{"N/A"};

    test.check(
      mgr.get(&greeting::expression, "john1", na) == "Hi", "john1 greeting 1");
    test.check(
      mgr.get(&greeting::expression, "john2", na) == na, "john2 greeting 1");

    mgr.exchange<greeting>("john1", "john2");

    test.check(
      mgr.get(&greeting::expression, "john1", na) == na, "john1 greeting 2");
    test.check(
      mgr.get(&greeting::expression, "john2", na) == "Hi", "john2 greeting 2");

    test.check(
      mgr.get(&person::family_name, "john1", na) == na, "john1 name 1");
    test.check(
      mgr.get(&person::family_name, "john2", na) == "Roe", "john2 name 1");

    mgr.exchange<person>("john1", "john2");

    test.check(
      mgr.get(&person::family_name, "john1", na) == "Roe", "john1 name 2");
    test.check(
      mgr.get(&person::family_name, "john2", na) == na, "john2 name 2");

    mgr.exchange<person, greeting>("john1", "john2");

    test.check(
      mgr.get(&greeting::expression, "john1", na) == "Hi", "john1 greeting 3");
    test.check(
      mgr.get(&greeting::expression, "john2", na) == na, "john2 greeting 3");
    test.check(
      mgr.get(&person::family_name, "john1", na) == na, "john1 name 3");
    test.check(
      mgr.get(&person::family_name, "john2", na) == "Roe", "john2 name 3");
}
//------------------------------------------------------------------------------
// for-single
//------------------------------------------------------------------------------
void manager_component_for_single_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 13, "for-single"};

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
    eagitest::case_ test{s, 14, "for-each"};
    eagitest::track trck{test, 0, 3};

    std::map<eagine::identifier_t, std::tuple<std::string, std::string>> names;

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

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
// for-each 2
//------------------------------------------------------------------------------
void manager_component_for_each_2(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 15, "for-each 2"};
    eagitest::track trck{test, 0, 4};

    std::map<eagine::identifier_t, std::tuple<std::string, std::string>> names;

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

    std::map<std::string, std::string> greetings;

    const auto add =
      [&](
        auto eid, std::string name, std::string family_name, std::string expr) {
          greetings[family_name] = expr;
          mgr.add(
            eid,
            person(std::move(name), std::move(family_name)),
            greeting(std::move(expr)));
      };

    add(id_v("John"), "John", "Doe", "Hi");
    add(id_v("Jane"), "Jane", "Roe", "Hey");
    add(id_v("Jack"), "Jack", "Daniels", "Howdy");

    mgr.for_each_with<const person, const greeting>(
      [&](const auto, auto& p, auto& g) {
          test.check(
            greetings[p.read().family_name] == g.read().expression, "1");
          greetings[p.read().name] = g.read().expression;
          trck.checkpoint(1);
      });

    mgr.for_each_with<person, greeting>([&](const auto, auto& p, auto& g) {
        test.check(greetings[p.write().name] == g.write().expression, "2");
        trck.checkpoint(2);
    });

    mgr.for_each_with<const person, greeting>(
      [&](const auto, auto& p, auto& g) {
          test.check(greetings[p.read().name] == g.read().expression, "3");
          g.write().expression = "How's going";
          trck.checkpoint(3);
      });

    mgr.for_each_with<person, const greeting>(
      [&](const auto e, auto& p, auto& g) {
          test.check(p.read().name == eagine::identifier(e).name().str(), "4");
          test.check(g.read().expression == "How's going", "5");
          trck.checkpoint(4);
      });
}
//------------------------------------------------------------------------------
// for-each 3
//------------------------------------------------------------------------------
void manager_component_for_each_3(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 16, "for-each opt"};
    eagitest::track trck{test, 0, 4};
    eagitest::track people{test, 3, 1};
    eagitest::track greetings{test, 3, 1};
    eagitest::track both{test, 2, 1};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

    mgr.add(id_v("John"), person("John", "Doe"));
    mgr.add(id_v("Jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("Bill"), person("Bill", "Roe"), greeting("Hello"));
    mgr.add(id_v("Jack"), greeting("Howdy"));

    mgr.for_each_opt<const person, const greeting>(
      {eagine::construct_from,
       [&](
         eagine::identifier_t,
         eagine::ecs::manipulator<const person>& p,
         eagine::ecs::manipulator<const greeting>& g) {
           if(p.has_value()) {
               trck.checkpoint(1);
               people.checkpoint(1);
               test.check(not p.read().name.empty(), "has name");
           }
           if(g.has_value()) {
               trck.checkpoint(2);
               greetings.checkpoint(1);
               test.check(not g.read().expression.empty(), "has greeting");
           }
       }});

    mgr.for_each_opt<person, greeting>(
      {eagine::construct_from,
       [&](
         eagine::identifier_t e,
         eagine::ecs::manipulator<person>& p,
         eagine::ecs::manipulator<greeting>& g) {
           if(p.has_value()) {
               trck.checkpoint(3);
               test.check(not p.write().name.empty(), "has name");
               test.check(
                 p.read().name == eagine::identifier(e).name().str(),
                 "name match");
           }
           if(g.has_value()) {
               trck.checkpoint(4);
               test.check(not g.write().expression.empty(), "has greeting");
           }
       }});

    mgr.for_each_opt<person, const greeting>(
      {eagine::construct_from,
       [&](
         eagine::identifier_t e,
         eagine::ecs::manipulator<person>& p,
         eagine::ecs::manipulator<const greeting>& g) {
           if(p.has_value() and g.has_value()) {
               both.checkpoint(1);
               test.check(not g.read().expression.empty(), "has greeting");
               test.check(not p.read().name.empty(), "has name");
               test.check(
                 p.write().name == eagine::identifier(e).name().str(),
                 "name match");
           }
       }});
}
//------------------------------------------------------------------------------
// for-each 4
//------------------------------------------------------------------------------
void manager_component_for_each_4(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 17, "for-each with opt"};
    eagitest::track trck{test, 0, 4};
    eagitest::track people{test, 3, 1};
    eagitest::track greetings{test, 3, 1};
    eagitest::track both{test, 2, 1};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

    mgr.add(id_v("John"), person("John", "Doe"));
    mgr.add(id_v("Jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("Bill"), person("Bill", "Roe"), greeting("Hello"));
    mgr.add(id_v("Jack"), greeting("Howdy"));

    mgr.for_each_with_opt<const person, const greeting>(
      [&](auto, auto& p, auto& g) {
          if(p.has_value()) {
              trck.checkpoint(1);
              people.checkpoint(1);
              test.check(not p.read().name.empty(), "has name");
          }
          if(g.has_value()) {
              trck.checkpoint(2);
              greetings.checkpoint(1);
              test.check(not g.read().expression.empty(), "has greeting");
          }
      });

    mgr.for_each_with_opt<person, greeting>([&](auto e, auto& p, auto& g) {
        if(p.has_value()) {
            trck.checkpoint(3);
            test.check(not p.write().name.empty(), "has name");
            test.check(
              p.read().name == eagine::identifier(e).name().str(),
              "name match");
        }
        if(g.has_value()) {
            trck.checkpoint(4);
            test.check(not g.write().expression.empty(), "has greeting");
        }
    });

    mgr.for_each_with_opt<person, const greeting>(
      [&](auto e, auto& p, auto& g) {
          if(p.has_value() and g.has_value()) {
              both.checkpoint(1);
              test.check(not g.read().expression.empty(), "has greeting");
              test.check(not p.write().name.empty(), "has name");
              test.check(
                p.read().name == eagine::identifier(e).name().str(),
                "name match");
          }
      });
}
//------------------------------------------------------------------------------
// for-each 5
//------------------------------------------------------------------------------
void manager_component_for_each_5(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 18, "for-each with opt 2"};
    eagitest::track trck{test, 0, 4};
    eagitest::track people{test, 3, 1};
    eagitest::track greetings{test, 3, 1};
    eagitest::track both{test, 2, 1};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();

    mgr.add(id_v("John"), person("John", "Doe"));
    mgr.add(id_v("Jane"), person("Jane", "Doe"), greeting("Hi"));
    mgr.add(id_v("Bill"), person("Bill", "Roe"), greeting("Hello"));
    mgr.add(id_v("Jack"), greeting("Howdy"));

    mgr.for_each_with_opt<const person, const greeting>(
      [&](auto, auto& p, auto& g) {
          if(p.has_value()) {
              trck.checkpoint(1);
              people.checkpoint(1);
              test.check(not p->name.empty(), "has name");
              test.check(
                p.read(&person::family_name).has_value(), "has family name");
          }
          if(g.has_value()) {
              trck.checkpoint(2);
              greetings.checkpoint(1);
              test.check(not g->expression.empty(), "has greeting");
          }
      });

    mgr.for_each_with_opt<person, greeting>([&](auto e, auto& p, auto& g) {
        if(p.has_value()) {
            trck.checkpoint(3);
            test.check(not p.write().name.empty(), "has name");
            test.check(
              p.write(&person::family_name).has_value(), "has family name");
            test.check(
              p->name == eagine::identifier(e).name().str(), "name match");
        }
        if(g.has_value()) {
            trck.checkpoint(4);
            test.check(
              g.write(&greeting::expression).has_value(), "has greeting");
        }
    });

    mgr.for_each_with_opt<person, const greeting>(
      [&](auto e, auto& p, auto& g) {
          if(p.has_value() and g.has_value()) {
              both.checkpoint(1);
              test.check(
                g.read(&greeting::expression)
                  .transform([&](const auto& expr) { return not expr.empty(); })
                  .or_false(),
                "has greeting");

              test.check(
                p.read(&person::family_name)
                  .transform([&](const auto& name) { return not name.empty(); })
                  .or_false(),
                "has family name");

              test.check(
                p.write(&person::name)
                  .transform([&](const auto& name) {
                      return name == eagine::identifier(e).name().str();
                  })
                  .or_false(),
                "name match");
          }
      });
}
//------------------------------------------------------------------------------
// has / has-all
//------------------------------------------------------------------------------
void manager_component_has_1(auto& s) {
    eagitest::case_ test{s, 19, "has"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
    eagitest::case_ test{s, 20, "show/hide"};
    eagitest::track trck{test, 0, 2};

    eagine::ecs::basic_manager<eagine::identifier_t> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr
      .register_component_storage<eagine::ecs::chunk_map_cmp_storage, greeting>();

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
    eagitest::case_ test{s, 21, "relation/has"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, father>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, mother>();

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
// remove relation
//------------------------------------------------------------------------------
void manager_component_remove_relation_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 22, "remove relation"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, father>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, mother>();

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

    mgr.remove_relation<mother>("vader", "shmi");
    test.check(mgr.has<father>("luke", "vader"), "9");
    test.check(mgr.has<father>("leia", "vader"), "10");
    test.check(mgr.has<mother>("luke", "padme"), "11");
    test.check(mgr.has<mother>("leia", "padme"), "12");
    test.check(not mgr.has<mother>("vader", "shmi"), "13");
    test.check(mgr.has<father>("vader", "force"), "14");
    test.check(mgr.has<mother>("angryguy", "leia"), "15");
    test.check(mgr.has<father>("angryguy", "hans"), "16");

    mgr.remove_relation<mother>("luke", "padme");
    mgr.remove_relation<mother>("leia", "padme");

    test.check(mgr.has<father>("luke", "vader"), "17");
    test.check(mgr.has<father>("leia", "vader"), "18");
    test.check(not mgr.has<mother>("luke", "padme"), "19");
    test.check(not mgr.has<mother>("leia", "padme"), "20");
    test.check(not mgr.has<mother>("vader", "shmi"), "21");
    test.check(mgr.has<father>("vader", "force"), "22");
    test.check(mgr.has<mother>("angryguy", "leia"), "23");
    test.check(mgr.has<father>("angryguy", "hans"), "24");

    mgr.remove_relation<father>("luke", "vader");
    mgr.remove_relation<father>("leia", "vader");

    test.check(not mgr.has<father>("luke", "vader"), "25");
    test.check(not mgr.has<father>("leia", "vader"), "26");
    test.check(not mgr.has<mother>("luke", "padme"), "27");
    test.check(not mgr.has<mother>("leia", "padme"), "28");
    test.check(not mgr.has<mother>("vader", "shmi"), "29");
    test.check(mgr.has<father>("vader", "force"), "30");
    test.check(mgr.has<mother>("angryguy", "leia"), "31");
    test.check(mgr.has<father>("angryguy", "hans"), "32");
}
//------------------------------------------------------------------------------
// clear
//------------------------------------------------------------------------------
void manager_component_clear_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 23, "clear"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, father>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, mother>();

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

    test.check(mgr.has<person>("force"), "person force");
    test.check(mgr.has<person>("luke"), "person luke");
    test.check(mgr.has<person>("leia"), "person leia");
    test.check(mgr.has<person>("hans"), "person hans");
    test.check(mgr.has<person>("vader"), "person vader");
    test.check(mgr.has<person>("padme"), "person padme");
    test.check(mgr.has<person>("shmi"), "person shmi");
    test.check(mgr.has<person>("jarjar"), "person jarjar");
    test.check(mgr.has<person>("yoda"), "person yoda");
    test.check(mgr.has<person>("angryguy"), "person kylo");

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

    mgr.clear();

    test.check(not mgr.has<person>("force"), "not person force");
    test.check(not mgr.has<person>("luke"), "not person luke");
    test.check(not mgr.has<person>("leia"), "not person leia");
    test.check(not mgr.has<person>("hans"), "not person hans");
    test.check(not mgr.has<person>("vader"), "not person vader");
    test.check(not mgr.has<person>("padme"), "not person padme");
    test.check(not mgr.has<person>("shmi"), "not person shmi");
    test.check(not mgr.has<person>("jarjar"), "not person jarjar");
    test.check(not mgr.has<person>("yoda"), "not person yoda");
    test.check(not mgr.has<person>("angryguy"), "not person kylo");

    test.check(not mgr.has<father>("luke", "vader"), "9");
    test.check(not mgr.has<father>("leia", "vader"), "10");
    test.check(not mgr.has<mother>("luke", "padme"), "11");
    test.check(not mgr.has<mother>("leia", "padme"), "12");
    test.check(not mgr.has<mother>("vader", "shmi"), "13");
    test.check(not mgr.has<father>("vader", "force"), "14");
    test.check(not mgr.has<mother>("angryguy", "leia"), "15");
    test.check(not mgr.has<father>("angryguy", "hans"), "16");
}
//------------------------------------------------------------------------------
// select/cross
//------------------------------------------------------------------------------
void manager_component_select_cross_1(auto& s) {
    using eagine::id_v;
    eagitest::case_ test{s, 24, "select/cross"};

    eagine::ecs::basic_manager<std::string> mgr;
    mgr.register_component_storage<eagine::ecs::chunk_map_cmp_storage, person>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, father>();
    mgr.register_relation_storage<eagine::ecs::chunk_map_rel_storage, mother>();

    const auto run_tests =
      [&](int same, int diff, int reld, std::string_view label) {
          int csame{0};
          int cdiff{0};
          int creld{0};
          const auto do_tests = [&](auto e1, auto&, auto e2, auto&) {
              if(e1 == e2) {
                  ++csame;
              } else {
                  ++cdiff;
              }
              if(mgr.has<mother>(e1, e2) or mgr.has<father>(e1, e2)) {
                  ++creld;
              }
          };
          mgr.select<person>().cross<person>().for_each(do_tests);

          test.check_equal(same, csame, label);
          test.check_equal(diff, cdiff, label);
          test.check_equal(reld, creld, label);
      };

    mgr.ensure<person>("force").set("The", "Force");
    run_tests(1, 0, 0, "A");

    mgr.ensure<person>("shmi").set("Shmi", "Skywalker");
    run_tests(2, 2, 0, "B");

    mgr.ensure<person>("vader").set("Anakin", "Skywalker");
    mgr.ensure<mother>("vader", "shmi");
    run_tests(3, 6, 1, "C");

    mgr.ensure<person>("padme").set("Padme", "Amidala");
    mgr.ensure<father>("vader", "force");
    run_tests(4, 12, 2, "D");

    mgr.ensure<person>("luke").set("Luke", "Skywalker");
    mgr.ensure<person>("leia").set("Leia", "Organa");
    mgr.ensure<father>("leia", "vader");
    mgr.ensure<mother>("leia", "padme");
    mgr.ensure<father>("luke", "vader");
    mgr.ensure<mother>("luke", "padme");
    run_tests(6, 30, 6, "E");

    mgr.ensure<person>("jarjar").set("Jar-Jar", "Binks");
    run_tests(7, 42, 6, "F");

    mgr.ensure<person>("yoda").set("Yoda", "N/A");
    mgr.ensure<person>("hans").set("Hans", "Olo");
    mgr.ensure<person>("chewie").set("Chewbacca", "N/A");
    run_tests(10, 90, 6, "G");

    mgr.ensure<person>("angryguy").set("Kylo", "Ren");
    mgr.ensure<mother>("angryguy", "leia");
    mgr.ensure<father>("angryguy", "hans");
    run_tests(11, 110, 8, "H");
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "manager", 24};
    test.once(manager_component_register_1);
    test.once(manager_component_register_2);
    test.once(manager_component_write_has_1);
    test.once(manager_component_write_get_1);
    test.once(manager_component_write_read_1);
    test.once(manager_component_manipulator_1);
    test.once(manager_component_add_has_name_1);
    test.once(manager_component_add_remove_1);
    test.once(manager_component_add_forget_1);
    test.once(manager_component_add_copy_1);
    test.once(manager_component_add_exchange_1);
    test.once(manager_component_add_exchange_2);
    test.once(manager_component_for_single_1);
    test.once(manager_component_for_each_1);
    test.once(manager_component_for_each_2);
    test.once(manager_component_for_each_3);
    test.once(manager_component_for_each_4);
    test.once(manager_component_for_each_5);
    test.once(manager_component_has_1);
    test.once(manager_component_show_hide_1);
    test.once(manager_component_relation_has_1);
    test.once(manager_component_remove_relation_1);
    test.once(manager_component_clear_1);
    test.once(manager_component_select_cross_1);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
