/// @example eagine/ecs/002_space_opera.hpp
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

struct name_surname : ecs::component<"NameSurnme"> {
    std::string first_name;
    std::string family_name;
};

template <bool Const>
struct name_surname_manip : ecs::basic_manipulator<name_surname, Const> {
    using ecs::basic_manipulator<name_surname, Const>::basic_manipulator;

    auto get_first_name() const -> const std::string& {
        return this->read().first_name;
    }

    auto set_first_name(std::string s) -> auto& {
        this->write().first_name.assign(std::move(s));
        return *this;
    }

    auto has_family_name(const char* str) const -> bool {
        return this->is_valid() && this->read().family_name == str;
    }

    auto get_family_name() const -> const std::string& {
        return this->read().family_name;
    }

    auto set_family_name(std::string s) -> auto& {
        this->write().family_name.assign(std::move(s));
        return *this;
    }
};

struct father : ecs::relation<"Father"> {};
struct mother : ecs::relation<"Mother"> {};

namespace ecs {

template <bool Const>
struct get_manipulator<name_surname, Const> {
    using type = name_surname_manip<Const>;
};

} // namespace ecs

auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.cio().print(identifier{"ECS"}, "starting");

    ecs::basic_manager<std::string> sso;
    sso.register_component_storage<ecs::std_map_cmp_storage, name_surname>();
    sso.register_relation_storage<ecs::std_map_rel_storage, father>();
    sso.register_relation_storage<ecs::std_map_rel_storage, mother>();

    sso.add<name_surname>("force").set_first_name("The").set_family_name(
      "Force");
    sso.add<name_surname>("luke").set_first_name("Luke").set_family_name(
      "Skywalker");
    sso.add<name_surname>("leia").set_first_name("Leia").set_family_name(
      "Organa");
    sso.add<name_surname>("hans").set_first_name("Hans").set_family_name("Olo");
    sso.add<name_surname>("vader").set_first_name("Anakin").set_family_name(
      "Skywalker");
    sso.add<name_surname>("padme").set_first_name("Padme").set_family_name(
      "Amidala");
    sso.add<name_surname>("shmi").set_first_name("Shmi").set_family_name(
      "Skywalker");
    sso.add<name_surname>("jarjar").set_first_name("Jar-Jar").set_family_name(
      "Binks");
    sso.add<name_surname>("chewie").set_first_name("Chewbacca");
    sso.add<name_surname>("yoda").set_first_name("Yoda");
    sso.add<name_surname>("kilo").set_first_name("Kylo").set_family_name("Ren");

    sso.add<father>("luke", "vader");
    sso.add<father>("leia", "vader");
    sso.add<mother>("luke", "padme");
    sso.add<mother>("leia", "padme");
    sso.add<mother>("vader", "shmi");
    sso.add<father>("vader", "force");
    sso.add<mother>("kilo", "leia");
    sso.add<father>("kilo", "hans");

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

