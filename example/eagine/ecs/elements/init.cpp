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

import std;

namespace eagine {
//------------------------------------------------------------------------------
class elements_data_loader
  : public valtree::object_builder_impl<elements_data_loader> {
public:
    elements_data_loader(ecs::basic_manager<element_symbol>& elements) noexcept
      : _elements{elements} {}

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        assert(not path.empty());
        assert(not data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 2) {
            if(path.back() == "protons") {
                _elements.ensure<element_protons>(elem).set(
                  limit_cast<short>(data.front()));
            } else if(path.back() == "period") {
                _elements.ensure<element_period>(elem).set(
                  limit_cast<short>(data.front()));
            } else if(path.back() == "group") {
                _elements.ensure<element_group>(elem).set(
                  limit_cast<short>(data.front()));
            }
        } else if(path.size() == 4 and path[1] == "isotopes") {
            element_symbol isot{path[2]};
            if(path.back() == "neutrons") {
                _elements.ensure<isotope_neutrons>(isot).set(
                  limit_cast<short>(data.front()));
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        assert(not path.empty());
        assert(not data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 2) {
            if(path.back() == "atomic_weight") {
                _elements.ensure<atomic_weight>(elem).set(
                  limit_cast<float>(data.front()));
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        assert(not path.empty());
        assert(not data.empty());

        const element_symbol elem{path.front()};
        if(path.size() == 3) {
            if(path.back() == "latin") {
                _elements.ensure<element_name>(elem).set_latin_name(
                  to_string(data.front()));
            } else if(path.back() == "english") {
                _elements.ensure<element_name>(elem).set_english_name(
                  to_string(data.front()));
            }
        } else if(path.size() == 4 and path[1] == "isotopes") {
            element_symbol isot{path[2]};
            if(path.back() == "half_life") {
                data.and_then(from_string<std::chrono::duration<float>>)
                  .and_then([&](auto hl) -> noopt {
                      _elements.ensure<half_life>(isot).set(hl);
                      return {};
                  });
            }
        } else if(path.size() == 6 and path[1] == "isotopes") {
            if(path.back() == "latin") {
                _elements.ensure<element_name>(elem).set_latin_name(
                  to_string(data.front()));
            } else if(path.back() == "english") {
                _elements.ensure<element_name>(elem).set_english_name(
                  to_string(data.front()));
            }
        } else if(path.starts_with(_decay_path)) {
            element_symbol isot{path[2]};
            if(path.back() == "mode") {
                _elements.ensure<decay_modes>(isot).add(data.front());
            } else if(path.back() == "products") {
                _elements.ensure<decay_modes>(isot).back().and_then(
                  [&](auto& mode) -> noopt {
                      for(auto prod : data) {
                          mode.products.push_back(to_string(prod));
                      }
                      return {};
                  });
            }
        }
    }

    template <typename T>
    void do_add(const basic_string_path&, span<const T>) noexcept {}

    void add_object(const basic_string_path& path) noexcept final {
        if(path.size() == 3) {
            const element_symbol elem{path.front()};
            if(path[1] == "isotopes") {
                element_symbol isot{path[2]};
                _elements.ensure<isotope>(elem, isot);
            }
        }
    }

    void failed() noexcept final {
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
                      (dcy.products.empty() and not dcy_mode.is_fission) and
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
      std::make_shared<elements_data_loader>(elements),
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
