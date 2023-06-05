/// @example eagine/ecs/002_space_opera.hpp
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

struct name_surname : ecs::component<"NameSurnme"> {
    std::string first_name;
    std::string family_name;

    template <bool isConst>
    struct manipulator : ecs::basic_manipulator<name_surname, isConst> {
        using ecs::basic_manipulator<name_surname, isConst>::basic_manipulator;

        auto get_first_name() const -> const std::string& {
            return this->read().first_name;
        }

        auto set_first_name(std::string s) -> auto& {
            this->write().first_name.assign(std::move(s));
            return *this;
        }

        auto has_family_name(const char* str) const -> bool {
            return this->has_value() and this->read().family_name == str;
        }

        auto get_family_name() const -> const std::string& {
            return this->read().family_name;
        }

        auto set_family_name(std::string s) -> auto& {
            this->write().family_name.assign(std::move(s));
            return *this;
        }
    };
};

static_assert(ecs::component_with_manipulator<name_surname>);

struct father : ecs::relation<"Father"> {};
struct mother : ecs::relation<"Mother"> {};

auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.cio().print("ECS", "starting");

    ecs::basic_manager<std::string> space_soap_opera;
    space_soap_opera
      .register_component_storage<ecs::std_map_cmp_storage, name_surname>();
    space_soap_opera
      .register_relation_storage<ecs::std_map_rel_storage, father>();
    space_soap_opera
      .register_relation_storage<ecs::std_map_rel_storage, mother>();

    space_soap_opera.ensure<name_surname>("force")
      .set_first_name("The")
      .set_family_name("Force");
    space_soap_opera.ensure<name_surname>("luke")
      .set_first_name("Luke")
      .set_family_name("Skywalker");
    space_soap_opera.ensure<name_surname>("leia")
      .set_first_name("Leia")
      .set_family_name("Organa");
    space_soap_opera.ensure<name_surname>("hans")
      .set_first_name("Hans")
      .set_family_name("Olo");
    space_soap_opera.ensure<name_surname>("vader")
      .set_first_name("Anakin")
      .set_family_name("Skywalker");
    space_soap_opera.ensure<name_surname>("padme")
      .set_first_name("Padme")
      .set_family_name("Amidala");
    space_soap_opera.ensure<name_surname>("shmi")
      .set_first_name("Shmi")
      .set_family_name("Skywalker");
    space_soap_opera.ensure<name_surname>("jarjar")
      .set_first_name("Jar-Jar")
      .set_family_name("Binks");
    space_soap_opera.ensure<name_surname>("chewie").set_first_name("Chewbacca");
    space_soap_opera.ensure<name_surname>("yoda").set_first_name("Yoda");
    space_soap_opera.ensure<name_surname>("kilo")
      .set_first_name("Kylo")
      .set_family_name("Ren");

    space_soap_opera.ensure<father>("luke", "vader");
    space_soap_opera.ensure<father>("leia", "vader");
    space_soap_opera.ensure<mother>("luke", "padme");
    space_soap_opera.ensure<mother>("leia", "padme");
    space_soap_opera.ensure<mother>("vader", "shmi");
    space_soap_opera.ensure<father>("vader", "force");
    space_soap_opera.ensure<mother>("kilo", "leia");
    space_soap_opera.ensure<father>("kilo", "hans");

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

