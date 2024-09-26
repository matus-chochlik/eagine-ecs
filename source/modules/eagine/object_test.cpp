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
struct get_manipulator<::person, Const>
  : std::type_identity<::person_manipulator<Const>> {};

} // namespace eagine::ecs
//------------------------------------------------------------------------------
// ECS objects
//------------------------------------------------------------------------------
void object_create_destroy(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 1, "create & destroy"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;
    std::vector<eagine::identifier_value> entities;

    const std::size_t count{1024Z};
    objects.reserve(count);
    entities.reserve(count);

    eagine::identifier_value id_seq{};

    while(objects.size() < count) {
        id_seq = increment(id_seq);
        objects.emplace_back(id_seq);

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Hi";

        entities.push_back(obj.entity());
        test.check(m.knows(entities.back()), "knows");
        trck.checkpoint(1);
    }

    objects.clear();

    for(auto ent : entities) {
        test.check(not m.knows(ent), "does not know");
        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_destroy(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 2, "spawn & destroy"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;
    std::vector<eagine::identifier_value> entities;

    const std::size_t count{1024Z};
    objects.reserve(count);
    entities.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Bye";

        entities.push_back(obj.entity());
        test.check(m.knows(entities.back()), "knows");
        trck.checkpoint(1);
    }

    objects.clear();

    for(auto ent : entities) {
        test.check(not m.knows(ent), "does not know");
        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_ensure_has(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 3, "spawn & ensure & has"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Ahoy";

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(obj.has<greeting>(), "has");
        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_add_has(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 4, "spawn & add & has"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.add(greeting("Hi"));

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(obj.has<greeting>(), "has");
        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_ensure_remove(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 5, "spawn & ensure & remove"};
    eagitest::track trck{test, 0, 6};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<
      eagine::ecs::flat_map_cmp_storage,
      person,
      greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Ahoy";
        obj.ensure<person>().set("Marty", "McFly");

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(obj.has_all<person, greeting>(), "has");
        trck.checkpoint(2);
    }

    for(auto& obj : objects) {
        obj.remove<greeting>();
        trck.checkpoint(3);
    }

    for(auto& obj : objects) {
        test.check(not obj.has<greeting>(), "has not greeting");
        test.check(obj.has<person>(), "has person");
        trck.checkpoint(4);
    }

    for(auto& obj : objects) {
        obj.remove<person>();
        trck.checkpoint(5);
    }

    for(auto& obj : objects) {
        test.check(not obj.has<greeting>(), "has not greeting");
        test.check(not obj.has<person>(), "has not person");
        trck.checkpoint(6);
    }
}
//------------------------------------------------------------------------------
void object_spawn_ensure_forget(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 6, "spawn & ensure & forgeet"};
    eagitest::track trck{test, 0, 4};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Ahoy";

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(obj.has<greeting>(), "has");
        trck.checkpoint(2);
    }

    std::vector<eagine::identifier_value> entities;
    entities.reserve(count);

    for(auto& obj : objects) {
        entities.emplace_back(obj.entity());
        obj.forget();
        trck.checkpoint(3);
    }

    for(auto ent : entities) {
        test.check(not m.knows(ent), "does not know");
        trck.checkpoint(4);
    }
}
//------------------------------------------------------------------------------
void object_spawn_hide_show(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 7, "spawn & hide & show"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{objects.back()};
        obj.ensure<greeting>()->expression = "Hey";

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(not obj.is_hidden<greeting>(), "is visible");
        obj.hide<greeting>();
        test.check(obj.is_hidden<greeting>(), "is hidden");
        obj.show<greeting>();
        test.check(not obj.is_hidden<greeting>(), "is shown");

        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_add_copy_from(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 8, "spawn & add & copy_from"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    objects.emplace_back(eagine::ecs::object::spawn(s.context()));
    auto& first{objects.back()};
    first.add(greeting("Ahoy"));

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));
        auto& obj{objects.back()};
        obj.copy_from<greeting>(objects.front());

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        test.check(obj.has<greeting>(), "has");

        trck.checkpoint(2);
    }
}
//------------------------------------------------------------------------------
void object_spawn_add_exchange_with(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 9, "spawn & add & exchange_with"};
    eagitest::track trck{test, 0, 3};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<eagine::ecs::flat_map_cmp_storage, greeting>();

    std::vector<eagine::ecs::object> src;
    std::vector<eagine::ecs::object> dst;

    const std::size_t count{1024Z};
    src.reserve(count);
    dst.reserve(count);

    while(src.size() < count) {
        src.emplace_back(eagine::ecs::object::spawn(s.context()));

        auto& obj{src.back()};
        obj.add(greeting("Hi"));

        trck.checkpoint(1);
    }

    for(auto& obj : src) {
        dst.emplace_back(eagine::ecs::object::spawn(s.context()));
        test.check(not dst.back().has<greeting>(), "dst has not");
        obj.exchange_with<greeting>(dst.back());
        test.check(not obj.has<greeting>(), "src has not");

        trck.checkpoint(2);
    }

    for(auto& obj : dst) {
        test.check(obj.has<greeting>(), "has");

        trck.checkpoint(3);
    }
}
//------------------------------------------------------------------------------
void object_spawn_add_write_read(eagitest::ctx_suite& s) {
    eagitest::case_ test{s, 10, "spawn & add & write & read"};
    eagitest::track trck{test, 0, 2};

    auto& m{enable_ecs(s.context()).value()};

    m.register_component_storages<
      eagine::ecs::flat_map_cmp_storage,
      person,
      greeting>();

    std::vector<eagine::ecs::object> objects;

    const std::size_t count{1024Z};
    objects.reserve(count);

    while(objects.size() < count) {
        objects.emplace_back(eagine::ecs::object::spawn(s.context()));
        auto& obj{objects.back()};
        obj.ensure<person>();
        obj.write<person>(
          [](eagine::ecs::object::entity_type, person_manipulator<false>& p) {
              p.set("Marty", "McFly");
          });

        trck.checkpoint(1);
    }

    for(auto& obj : objects) {
        obj.read<person>(
          [&test, &trck](
            eagine::ecs::object::entity_type, person_manipulator<true>& p) {
              test.check(p.has_name("Marty", "McFly"), "has");
              trck.checkpoint(2);
          });
    }
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "object", 10};
    test.once(object_create_destroy);
    test.once(object_spawn_destroy);
    test.once(object_spawn_ensure_has);
    test.once(object_spawn_add_has);
    test.once(object_spawn_ensure_remove);
    test.once(object_spawn_ensure_forget);
    test.once(object_spawn_hide_show);
    test.once(object_spawn_add_copy_from);
    test.once(object_spawn_add_exchange_with);
    test.once(object_spawn_add_write_read);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
