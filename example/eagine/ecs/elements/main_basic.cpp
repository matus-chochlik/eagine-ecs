/// @example eagine/ecs/elements/main_basic.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "components.hpp"
#include "entity.hpp"
#include "init.hpp"
#include "relations.hpp"
#include <eagine/ecs/basic_manager.hpp>
#include <eagine/ecs/cmp_storage.hpp>
#include <eagine/ecs/entity/string.hpp>
#include <eagine/ecs/rel_storage.hpp>
#include <eagine/ecs/storage_caps.hpp>
#include <eagine/logging/logger.hpp>
#include <eagine/main.hpp>
#include <iostream>

namespace eagine {
//------------------------------------------------------------------------------
// Usage
//------------------------------------------------------------------------------
static void
print_elements_with_english_name(ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const element_name>(
      [](const auto& sym, ecs::manipulator<const element_name>& name) {
          if(name.has_english_name()) {
              std::cout << sym << ": " << name.get_english_name() << std::endl;
          }
      });
    std::cout << std::endl;
}
//------------------------------------------------------------------------------
static void
print_names_of_noble_gasses(ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const element_name, const element_group>(
      [](
        const auto&,
        ecs::manipulator<const element_name>& name,
        ecs::manipulator<const element_group>& group) {
          if(group.has_number(18)) {
              std::cout << name.get_latin_name() << std::endl;
          }
      });
    std::cout << std::endl;
}
//------------------------------------------------------------------------------
static void
print_names_of_actinides(ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with_opt<
      const element_name,
      const element_period,
      const element_group>([](
                             const auto&,
                             ecs::manipulator<const element_name>& name,
                             ecs::manipulator<const element_period>& period,
                             ecs::manipulator<const element_group>& group) {
        if(period.has_number(7) && group.has_number(3)) {
            std::cout << name.get_latin_name() << std::endl;
        }
    });
    std::cout << std::endl;
}
//------------------------------------------------------------------------------
static void
print_isotopes_of_hydrogen(ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const isotope_neutrons>(
      [&](const auto& isot, auto& neutrons) {
          if(elements.has<isotope>("H", isot)) {
              std::cout << isot << ": " << neutrons.read().number;
              if(elements.has<half_life>(isot)) {
                  std::cout << " (unstable)";
              }
              std::cout << std::endl;
          }
      });
    std::cout << std::endl;
}
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    ctx.log().info("starting");

    ecs::basic_manager<element_symbol> elements;
    initialize(ctx, elements);

    print_elements_with_english_name(elements);
    print_names_of_noble_gasses(elements);
    print_names_of_actinides(elements);
    print_isotopes_of_hydrogen(elements);

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine
