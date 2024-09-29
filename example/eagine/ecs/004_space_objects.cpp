/// @example eagine/ecs/004_space_objects.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.ecs;
import std;

namespace eagine {
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
struct father : ecs::relation<"Father"> {};
struct mother : ecs::relation<"Mother"> {};
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.cio().print("ECS", "starting");

    auto& space_soap_opera{enable_ecs(ctx).value()};

    space_soap_opera.register_component_storages<
      eagine::ecs::std_map_cmp_storage,
      name_surname>();
    space_soap_opera.register_relation_storages<
      eagine::ecs::std_map_rel_storage,
      father,
      mother>();

    struct subject : ecs::object {
        using ecs::object::object;

        auto set_name(std::string name, std::string family_name) -> subject& {
            ensure<name_surname>()
              .set_first_name(std::move(name))
              .set_family_name(std::move(family_name));
            return *this;
        }

        auto set_father(const subject& p) -> subject& {
            ensure<father>(p);
            return *this;
        }

        auto set_mother(const subject& m) -> subject& {
            ensure<mother>(m);
            return *this;
        }
    };

    subject force{"force"};
    force.set_name("The", "Force");

    subject leia{"leia"};
    leia.set_name("Leia", "Organa");

    subject luke{"luke"};
    luke.set_name("Luke", "Skywalker");

    subject hans{"hans"};
    hans.set_name("Hans", "Olo");

    subject vader{"vader"};
    vader.set_name("Anakin", "Skywalker");

    subject padme{"padme"};
    padme.set_name("Padme", "Amidala");

    subject shmi{"shmi"};
    padme.set_name("Shmi", "Skywalker");

    subject jarjar{"jarjar"};
    padme.set_name("Jar-Jar", "Binks");

    subject kilo{"kilo"};
    padme.set_name("Kylo", "Ren");

    leia.set_father(vader);
    leia.set_mother(padme);
    luke.set_father(vader);
    luke.set_mother(padme);
    vader.set_father(force);
    vader.set_mother(shmi);
    kilo.set_father(hans);
    kilo.set_mother(leia);

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

