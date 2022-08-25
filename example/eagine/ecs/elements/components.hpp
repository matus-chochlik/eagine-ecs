/// @example eagine/ecs/elements/components.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_EXAMPLE_ECS_COMPONENTS_HPP // NOLINT(llvm-header-guard)
#define EAGINE_EXAMPLE_ECS_COMPONENTS_HPP

#include "decay_modes.hpp"
#include "entity.hpp"
#if EAGINE_ECS_MODULE
import <chrono>;
import <vector>;
#else
#include <eagine/ecs/component.hpp>
#include <eagine/ecs/manipulator.hpp>
#include <eagine/identifier.hpp>
#include <chrono>
#include <vector>
#endif

namespace eagine {
//------------------------------------------------------------------------------
struct element_name : ecs::component<"Name"> {
    std::string latin;
    std::string english;
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<element_name, Const> {
    struct type : basic_manipulator<element_name, Const> {
        using basic_manipulator<element_name, Const>::basic_manipulator;

        auto set_names(std::string lat, std::string eng) -> auto& {
            this->write().latin = std::move(lat);
            this->write().english = std::move(eng);
            return *this;
        }

        auto get_latin_name() const -> const std::string& {
            return this->read().latin;
        }

        auto get_english_name() const -> const std::string& {
            const auto& name = this->read().english;
            if(name.empty()) {
                return this->read().latin;
            }
            return name;
        }

        auto has_english_name() const -> bool {
            return this->is_valid() ? !this->read().english.empty() : false;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct element_protons : ecs::component<"Protons"> {
    short number{0};
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<element_protons, Const> {
    struct type : basic_manipulator<element_protons, Const> {
        using basic_manipulator<element_protons, Const>::basic_manipulator;

        auto set(const short number) -> auto& {
            this->write().number = number;
            return *this;
        }

        auto has_number(const short number) const -> bool {
            return this->is_valid() ? this->read().number == number : false;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct isotope_neutrons : ecs::component<"Neutrons"> {
    short number{0};
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<isotope_neutrons, Const> {
    struct type : basic_manipulator<isotope_neutrons, Const> {
        using basic_manipulator<isotope_neutrons, Const>::basic_manipulator;

        auto set(const short number) -> auto& {
            this->write().number = number;
            return *this;
        }

        auto has_number(const short number) const -> bool {
            return this->is_valid() ? this->read().number == number : false;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct element_period : ecs::component<"Period"> {
    short number{0};
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<element_period, Const> {
    struct type : basic_manipulator<element_period, Const> {
        using basic_manipulator<element_period, Const>::basic_manipulator;

        auto set(const short number) -> auto& {
            this->write().number = number;
            return *this;
        }

        auto has_number(const short number) const -> bool {
            return this->is_valid() ? this->read().number == number : false;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct element_group : ecs::component<"Group"> {
    short number{0};
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<element_group, Const> {
    struct type : basic_manipulator<element_group, Const> {
        using basic_manipulator<element_group, Const>::basic_manipulator;

        auto set(const short number) -> auto& {
            this->write().number = number;
            return *this;
        }

        auto has_number(const short number) const -> bool {
            return this->is_valid() ? this->read().number == number : false;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct atomic_weight : ecs::component<"AtomWeight"> {
    float value{0.F};
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<atomic_weight, Const> {
    struct type : basic_manipulator<atomic_weight, Const> {
        using basic_manipulator<atomic_weight, Const>::basic_manipulator;

        auto set(const float value) -> auto& {
            this->write().value = value;
            return *this;
        }

        auto get() const -> float {
            return this->read().value;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct half_life : ecs::component<"HalfLife"> {
    std::chrono::duration<float> time_seconds;
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<half_life, Const> {
    struct type : basic_manipulator<half_life, Const> {
        using basic_manipulator<half_life, Const>::basic_manipulator;

        template <typename R, typename P>
        auto set(const std::chrono::duration<R, P> value) -> auto& {
            this->write().time_seconds = value;
            return *this;
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
struct decay {
    std::vector<element_symbol> products;
};
//------------------------------------------------------------------------------
class decay_modes : public ecs::component<"DecayModes"> {
public:
    auto add(string_view symbol) -> decay* {
        int nv = 0;
        if(const auto nid{known_decay_modes::get_id(symbol)}) {
            for(const auto& [id, v, info] : _modes) {
                if(nid == id) {
                    nv = nv + 1;
                }
            }
            _modes.push_back({nid, nv, {}});
            return &std::get<2>(_modes.back());
        }
        return nullptr;
    }

    template <typename Function>
    void for_each(const Function& func) {
        for(auto& [id, v, info] : _modes) {
            (void)(v);
            if(auto mode_info{known_decay_modes::get_info(id)}) {
                func(*mode_info, info);
            }
        }
    }

    template <typename Function>
    void for_each(const Function& func) const {
        for(const auto& [id, v, info] : _modes) {
            (void)(v);
            if(auto mode_info{known_decay_modes::get_info(id)}) {
                func(*mode_info, info);
            }
        }
    }

private:
    std::vector<std::tuple<identifier_t, int, decay>> _modes;
};
//------------------------------------------------------------------------------
namespace ecs {
template <bool Const>
struct get_manipulator<decay_modes, Const> {
    struct type : basic_manipulator<decay_modes, Const> {
        using basic_manipulator<decay_modes, Const>::basic_manipulator;

        auto add(const string_view symbol) -> decay* {
            return this->write().add(symbol);
        }
    };
};
} // namespace ecs
//------------------------------------------------------------------------------
} // namespace eagine

#endif
