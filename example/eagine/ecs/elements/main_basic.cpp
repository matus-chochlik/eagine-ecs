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
#if !EAGINE_ECS_MODULE
#include <eagine/console/console.hpp>
#include <eagine/ecs/basic_manager.hpp>
#include <eagine/ecs/cmp_storage.hpp>
#include <eagine/ecs/entity/string.hpp>
#include <eagine/ecs/rel_storage.hpp>
#include <eagine/ecs/storage_caps.hpp>
#include <eagine/main_ctx.hpp>
#endif

namespace eagine {
//------------------------------------------------------------------------------
// Usage
//------------------------------------------------------------------------------
static void print_elements_with_english_name(
  const console& cio,
  ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const element_name>(
      [&](const auto& sym, const auto& name) {
          if(name.has_english_name()) {
              cio.print(identifier{"ECS"}, "${sym}: ${name}")
                .arg(identifier{"sym"}, sym)
                .arg(identifier{"name"}, name.get_english_name());
          }
      });
}
//------------------------------------------------------------------------------
static void print_names_of_noble_gasses(
  const console& cio,
  ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const element_name, const element_group>(
      [&](const auto&, const auto& name, const auto& group) {
          if(group.has_number(18)) {
              cio.print(identifier{"ECS"}, "${name}")
                .arg(identifier{"name"}, name.get_latin_name());
          }
      });
}
//------------------------------------------------------------------------------
static void print_names_of_actinides(
  const console& cio,
  ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with_opt<
      const element_name,
      const element_period,
      const element_group>([&](
                             const auto&,
                             ecs::manipulator<const element_name>& name,
                             ecs::manipulator<const element_period>& period,
                             ecs::manipulator<const element_group>& group) {
        if(period.has_number(7)) {
            auto opt_grp{group.read(&element_group::number)};
            if(opt_grp.value_or(3) == 3) {
                cio.print(identifier{"ECS"}, "${name}")
                  .arg(identifier{"name"}, name.get_latin_name());
            }
        }
    });
}
//------------------------------------------------------------------------------
static void print_isotopes_of_hydrogen(
  const console& cio,
  ecs::basic_manager<element_symbol>& elements) {

    elements.for_each_with<const isotope_neutrons>(
      [&](const auto& isot, auto& neutrons) {
          if(elements.has<isotope>("H", isot)) {
              string_view stabil;
              if(elements.has<half_life>(isot)) {
                  stabil = " (unstable)";
              }
              cio
                .print(identifier{"ECS"}, "${isotope}: ${neutrons}${stability}")
                .arg(identifier{"isotope"}, isot)
                .arg(identifier{"neutrons"}, neutrons.read().number)
                .arg(identifier{"stability"}, stabil);
          }
      });
}
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    const auto& cio = ctx.cio();
    cio.print(identifier{"ECS"}, "starting");

    ecs::basic_manager<element_symbol> elements;
    initialize(ctx, elements);

    print_elements_with_english_name(cio, elements);
    print_names_of_noble_gasses(cio, elements);
    print_names_of_actinides(cio, elements);
    print_isotopes_of_hydrogen(cio, elements);

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}
