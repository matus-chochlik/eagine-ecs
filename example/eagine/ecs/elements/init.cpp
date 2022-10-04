/// @example eagine/ecs/elements/init.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "init.hpp"
#include "components.hpp"
#include "entity.hpp"
#include "relations.hpp"
#include <cassert>

import <concepts>;

namespace eagine {
//------------------------------------------------------------------------------
class elements_data_loader
  : public valtree::object_builder_impl<elements_data_loader> {
public:
    elements_data_loader(ecs::basic_manager<element_symbol>& elements) noexcept
      : _elements{elements} {}

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) {
        assert(!path.empty());
        assert(!data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 2) {
            if(path.back() == "protons") {
                _elements.add<element_protons>(elem).set(
                  limit_cast<short>(extract(data)));
            } else if(path.back() == "period") {
                _elements.add<element_period>(elem).set(
                  limit_cast<short>(extract(data)));
            } else if(path.back() == "group") {
                _elements.add<element_group>(elem).set(
                  limit_cast<short>(extract(data)));
            }
        } else if(path.size() == 4 && path[1] == "isotopes") {
            element_symbol isot{path[2]};
            if(path.back() == "neutrons") {
                _elements.add<isotope_neutrons>(isot).set(
                  limit_cast<short>(extract(data)));
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) {
        assert(!path.empty());
        assert(!data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 2) {
            if(path.back() == "atomic_weight") {
                _elements.add<atomic_weight>(elem).set(
                  limit_cast<float>(extract(data)));
            }
        }
    }

    void do_add(const basic_string_path& path, span<const string_view> data) {
        assert(!path.empty());
        assert(!data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 3) {
            if(path.back() == "latin") {
                _elements.add<element_name>(elem).set_latin_name(
                  to_string(data.front()));
            } else if(path.back() == "english") {
                _elements.add<element_name>(elem).set_english_name(
                  to_string(data.front()));
            }
        } else if(path.size() == 4 && path[1] == "isotopes") {
            element_symbol isot{path[2]};
            if(path.back() == "half_life") {
                using hl_t = std::chrono::duration<float>;
                if(const auto hl{from_string<hl_t>(extract(data))}) {
                    _elements.add<half_life>(isot).set(extract(hl));
                }
            }
        } else if(path.size() == 6 && path[1] == "isotopes") {
            if(path.back() == "latin") {
                _elements.add<element_name>(elem).set_latin_name(
                  to_string(data.front()));
            } else if(path.back() == "english") {
                _elements.add<element_name>(elem).set_english_name(
                  to_string(data.front()));
            }
        } else if(path.starts_with(_decay_path)) {
            element_symbol isot{path[2]};
            if(path.back() == "mode") {
                _elements.add<decay_modes>(isot).add(extract(data));
            } else if(path.back() == "products") {
                if(const auto mode{_elements.add<decay_modes>(isot).back()}) {
                    for(auto prod : data) {
                        extract(mode).products.push_back(to_string(prod));
                    }
                }
            }
        }
    }

    template <typename T>
    void do_add(const basic_string_path&, span<const T>) {}

    void add_object(const basic_string_path& path) final {
        if(path.size() == 3) {
            const element_symbol elem{path.front()};
            if(path[1] == "isotopes") {
                element_symbol isot{path[2]};
                _elements.add<isotope>(elem, isot);
            }
        }
    }

    void failed() final {
        _elements.clear();
    }

private:
    ecs::basic_manager<element_symbol>& _elements;
    const basic_string_path _decay_path{"*/isotopes/*/decay/_"};
};
//------------------------------------------------------------------------------
static void cache_decay_products(ecs::basic_manager<element_symbol>& elements) {

    // for each original isotope with neutron count and some decay modes
    elements.for_each_with<const isotope_neutrons, decay_modes>(
      [&](auto& orig_is, auto& orig_nc, auto& modes) {
          // for each product isotope
          elements.for_each_with<const isotope_neutrons>(
            [&](auto& prod_is, auto& prod_nc) {
                // for each decay mode of the original isotope
                modes->for_each([&](auto& dcy_mode, auto& dcy) {
                    // if the isotope neutron count after the decay matches
                    if(
                      (dcy.products.empty() && !dcy_mode.is_fission) &&
                      (orig_nc->number + dcy_mode.neutron_count_diff ==
                       prod_nc->number)) {
                        // for each original element with proton count
                        elements.for_each_with<const element_protons>(
                          [&](auto& orig_el, auto& orig_pc) {
                              // if the original element has the original isotope
                              if(elements.has<isotope>(orig_el, orig_is)) {
                                  // for each product element
                                  elements.for_each_with<const element_protons>(
                                    [&](auto& prod_el, auto& prod_pc) {
                                        // if the element proton count after the
                                        // decay matches
                                        if(
                                          orig_pc->number +
                                            dcy_mode.proton_count_diff ==
                                          prod_pc->number) {
                                            // if the product element has the
                                            // product isotope
                                            if(elements.has<isotope>(
                                                 prod_el, prod_is)) {
                                                // cache the product isotope in
                                                // the original isotope decay info
                                                dcy.products.push_back(prod_is);
                                            }
                                        }
                                    });
                              }
                          });
                    }
                });
            });
      });
}
//------------------------------------------------------------------------------
static void do_initialize(
  main_ctx& ctx,
  ecs::basic_manager<element_symbol>& elements,
  const embedded_resource& json_res) {
    // components
    elements
      .register_component_storage<ecs::std_map_cmp_storage, element_name>();
    elements
      .register_component_storage<ecs::std_map_cmp_storage, element_protons>();
    elements
      .register_component_storage<ecs::std_map_cmp_storage, isotope_neutrons>();
    elements
      .register_component_storage<ecs::std_map_cmp_storage, element_period>();
    elements
      .register_component_storage<ecs::std_map_cmp_storage, element_group>();
    elements
      .register_component_storage<ecs::std_map_cmp_storage, atomic_weight>();
    elements.register_component_storage<ecs::std_map_cmp_storage, half_life>();
    elements.register_component_storage<ecs::std_map_cmp_storage, decay_modes>();
    // relations
    elements.register_relation_storage<ecs::std_map_rel_storage, isotope>();

    auto input{valtree::traverse_json_stream(
      valtree::make_building_value_tree_visitor(
        std::make_shared<elements_data_loader>(elements)),
      64,
      ctx.buffers(),
      ctx.log())};
    json_res.fetch(ctx, input.get_handler());

    cache_decay_products(elements);
}
//------------------------------------------------------------------------------
void initialize(main_ctx& ctx, ecs::basic_manager<element_symbol>& elements) {
    do_initialize(ctx, elements, embed<"ElemJSON">("elements.json"));
}
//------------------------------------------------------------------------------
} // namespace eagine
