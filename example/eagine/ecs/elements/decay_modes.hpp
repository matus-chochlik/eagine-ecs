/// @example eagine/ecs/elements/decay_modes.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_EXAMPLE_ECS_DECAY_MODES_HPP // NOLINT(llvm-header-guard)
#define EAGINE_EXAMPLE_ECS_DECAY_MODES_HPP

#include "entity.hpp"
#if EAGINE_ECS_MODULE
import <type_traits>;
#else
#include <eagine/flat_map.hpp>
#include <eagine/identifier.hpp>
#include <eagine/mp_list.hpp>
#include <type_traits>
#endif

namespace eagine {
//------------------------------------------------------------------------------
enum class decay_part {
    alpha,
    beta_p,
    beta_m,
    proton_emi,
    neutron_emi,
    electron_cap,
    fission,
    transition
};
//------------------------------------------------------------------------------
template <decay_part... M>
struct decay_mode_t {};
//------------------------------------------------------------------------------
template <decay_part... M>
static constexpr const auto is_fission_v =
  (false || ... || (M == decay_part::fission));
//------------------------------------------------------------------------------
struct decay_mode_info {
    std::string symbol;
    int proton_count_diff{0};
    int neutron_count_diff{0};
    bool is_fission{false};
};
//------------------------------------------------------------------------------
template <typename T>
struct decay_mode_traits;
//------------------------------------------------------------------------------
template <decay_part... M>
static inline auto mode_info(decay_mode_t<M...> = {})
  -> const decay_mode_info& {
    return decay_mode_traits<decay_mode_t<M...>>::info();
}
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::alpha>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"α", -2, -2, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::beta_p>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"β⁺", -1, 1, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::beta_m>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"β⁻", 1, -1, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::proton_emi>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"p⁺", -1, 0, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::neutron_emi>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"n⁰", 0, -1, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::electron_cap>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"+e⁻", -1, 1, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::fission>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"≺", 0, 0, true};
        return i;
    }
};
//------------------------------------------------------------------------------
template <>
struct decay_mode_traits<decay_mode_t<decay_part::transition>> {
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{"IT", 0, 0, false};
        return i;
    }
};
//------------------------------------------------------------------------------
template <decay_part... P>
struct decay_mode_traits<decay_mode_t<P...>> {
public:
    static auto info() noexcept -> const auto& {
        static const decay_mode_info i{
          _make_symbol(),
          (0 + ... + mode_info(decay_mode_t<P>{}).proton_count_diff),
          (0 + ... + mode_info(decay_mode_t<P>{}).neutron_count_diff),
          is_fission_v<P...>};
        return i;
    }

private:
    template <decay_part H>
    static void _append_symbol(decay_mode_t<H> m, std::string& s) {
        s.append(mode_info(m).symbol);
    }

    template <decay_part H, decay_part N, decay_part... T>
    static void _append_symbol(decay_mode_t<H, N, T...>, std::string& s) {
        s.append(mode_info(decay_mode_t<H>{}).symbol);
        s.append(",");
        _append_symbol(decay_mode_t<N, T...>{}, s);
    }

    static auto _make_symbol() noexcept -> std::string {
        std::string s;
        _append_symbol(decay_mode_t<P...>{}, s);
        return s;
    }
};
//------------------------------------------------------------------------------
template <typename DecayMode>
struct decay_mode_id;

template <>
struct decay_mode_id<decay_mode_t<decay_part::alpha>>
  : selector<id_v("AlphaDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::proton_emi>>
  : selector<id_v("PrtnEmissn")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::neutron_emi>>
  : selector<id_v("NtrnEmissn")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::electron_cap>>
  : selector<id_v("ElnCapDcy")> {};

template <>
struct decay_mode_id<
  decay_mode_t<decay_part::electron_cap, decay_part::electron_cap>>
  : selector<id_v("2ElnCapDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::electron_cap, decay_part::fission>>
  : selector<id_v("ElnCapFisn")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_m>>
  : selector<id_v("BetaMDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_m, decay_part::beta_m>>
  : selector<id_v("BetaM2Dcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_m, decay_part::alpha>>
  : selector<id_v("BtaMAlpDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_m, decay_part::neutron_emi>>
  : selector<id_v("BetaMNDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<
  decay_part::beta_m,
  decay_part::neutron_emi,
  decay_part::neutron_emi>> : selector<id_v("BetaMN2Dcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_p>>
  : selector<id_v("BetaPDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_p, decay_part::beta_p>>
  : selector<id_v("BetaP2Dcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::beta_p, decay_part::alpha>>
  : selector<id_v("BtaPAlpDcy")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::fission>>
  : selector<id_v("Fission")> {};

template <>
struct decay_mode_id<decay_mode_t<decay_part::transition>>
  : selector<id_v("Transition")> {};
//------------------------------------------------------------------------------
struct known_decay_modes {
    using m = decay_part;
    using list = mp_list<
      decay_mode_t<m::alpha>,
      decay_mode_t<m::proton_emi>,
      decay_mode_t<m::neutron_emi>,
      decay_mode_t<m::electron_cap>,
      decay_mode_t<m::electron_cap, m::electron_cap>,
      decay_mode_t<m::electron_cap, m::fission>,
      decay_mode_t<m::beta_m>,
      decay_mode_t<m::beta_m, m::beta_m>,
      decay_mode_t<m::beta_m, m::alpha>,
      decay_mode_t<m::beta_m, m::neutron_emi>,
      decay_mode_t<m::beta_m, m::neutron_emi, m::neutron_emi>,
      decay_mode_t<m::beta_p>,
      decay_mode_t<m::beta_p, m::beta_p>,
      decay_mode_t<m::beta_p, m::alpha>,
      decay_mode_t<m::fission>,
      decay_mode_t<m::transition>>;

    template <decay_part... M>
    static auto get_id(decay_mode_t<M...> = {}) noexcept {
        return decay_mode_id<decay_mode_t<M...>>::value;
    }

    static auto get_id(const string_view symbol) -> identifier_t {
        return _get_id(symbol, list{});
    }

    static auto get_info(const identifier_t mode_id) -> const decay_mode_info* {
        return _get_info(mode_id, list{});
    }

    static auto proton_count_diff(const identifier_t mode_id) noexcept -> int {
        if(const auto i{get_info(mode_id)}) {
            return extract(i).proton_count_diff;
        }
        return 0;
    }

    static auto neutron_count_diff(identifier_t mode_id) noexcept -> int {
        if(const auto i{get_info(mode_id)}) {
            return extract(i).neutron_count_diff;
        }
        return 0;
    }

private:
    static auto _get_info(identifier_t, mp_list<>) noexcept
      -> const decay_mode_info* {
        return nullptr;
    }

    template <typename H, typename... T>
    static auto _get_info(const identifier_t id, const mp_list<H, T...>)
      -> const decay_mode_info* {
        if(decay_mode_id<H>::value == id) {
            return &mode_info(H{});
        }
        return _get_info(id, mp_list<T...>{});
    }

    static auto _get_id(const string_view, const mp_list<>) noexcept
      -> identifier_t {
        return 0;
    }

    template <typename H, typename... T>
    static auto _get_id(const string_view symbol, const mp_list<H, T...>)
      -> identifier_t {
        if(are_equal(string_view(mode_info(H{}).symbol), symbol)) {
            return get_id(H{});
        }
        return _get_id(symbol, mp_list<T...>{});
    }
};
//------------------------------------------------------------------------------
} // namespace eagine

#endif
