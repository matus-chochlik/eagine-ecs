/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:manager;

import eagine.core.debug;
import eagine.core.types;
import eagine.core.string;
import eagine.core.utility;
import :entity_traits;
import :manipulator;
import :component;
import :storage;
import :cmp_storage;
import :rel_storage;
export import <memory>;
export import <string>;
import <stdexcept>;
import <type_traits>;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export [[noreturn]] void mgr_handle_cmp_is_reg(std::string&& c_name) {
    throw std::runtime_error(
      format("Component type '${1}' is already registered") %
      std::move(c_name));
}
//------------------------------------------------------------------------------
export [[noreturn]] void mgr_handle_cmp_not_reg(std::string&& c_name) {
    throw std::runtime_error(
      format("Component type '${1}' is not registered") % std::move(c_name));
}
//------------------------------------------------------------------------------
export template <typename Entity>
class basic_manager;
//------------------------------------------------------------------------------
export template <typename Entity, typename PL>
class component_relation;

export template <typename Entity, typename... PL>
class component_relation<Entity, mp_list<PL...>> {

public:
    component_relation(basic_manager<Entity>& m)
      : _m(m) {}

    template <typename... C>
    auto cross() -> component_relation<Entity, mp_list<PL..., mp_list<C...>>> {
        return {_m};
    }

    template <typename Func>
    void for_each(const Func& func) {
        _apply(_m, func, mp_list<PL...>());
    }

private:
    basic_manager<Entity>& _m;

    template <typename F, typename... C, typename... X>
    static auto _apply(
      basic_manager<Entity>& m,
      const F& func,
      const mp_list<mp_list<C...>>,
      X&&... x) {
        const auto wrap = [&func, &x...](
                            entity_param_t<Entity> e, manipulator<C>&... c) {
            func(std::forward<X>(x)..., e, c...);
        };
        m.for_each(
          callable_ref<void(entity_param_t<Entity>, manipulator<C> & ...)>{
            construct_from, wrap});
    }

    template <typename F, typename... C, typename L, typename... Ls, typename... X>
    static auto _apply(
      basic_manager<Entity>& m,
      const F& func,
      const mp_list<mp_list<C...>, L, Ls...>,
      X&&... x) {
        const auto wrap = [&m, &func, &x...](
                            entity_param_t<Entity> e, manipulator<C>&... c) {
            _apply(
              m, func, mp_list<L, Ls...>(), std::forward<X>(x)..., e, c...);
        };
        m.for_each(
          callable_ref<void(entity_param_t<Entity>, manipulator<C> & ...)>{
            wrap});
    }
};
//------------------------------------------------------------------------------
export template <typename Entity>
class basic_manager {
public:
    using entity_param = entity_param_t<Entity>;

    basic_manager() = default;

    template <typename Component>
    auto register_component_type(
      std::unique_ptr<component_storage<Entity, Component>>&& strg) -> auto& {
        _do_reg_stg_type<false>(
          _base_cmp_storage_ptr_t(std::move(strg)),
          Component::uid(),
          _cmp_name_getter<Component>());
        return *this;
    }

    template <typename Relation>
    auto register_relation_type(
      std::unique_ptr<relation_storage<Entity, Relation>>&& strg) -> auto& {
        _do_reg_stg_type<true>(
          _base_rel_storage_ptr_t(std::move(strg)),
          Relation::uid(),
          _cmp_name_getter<Relation>());
        return *this;
    }

    template <
      template <class, class>
      class Storage,
      typename Component,
      typename... P>
    auto register_component_storage(P&&... p) -> auto& {
        register_component_type<Component>(
          std::make_unique<Storage<Entity, Component>>(std::forward<P>(p)...));
        return *this;
    }

    template <
      template <class, class>
      class Storage,
      typename Relation,
      typename... P>
    auto register_relation_storage(P&&... p) -> auto& {
        register_relation_type<Relation>(
          std::make_unique<Storage<Entity, Relation>>(std::forward<P>(p)...));
        return *this;
    }

    template <typename Component>
    auto unregister_component_type() -> auto& {
        _do_unr_stg_type<false>(
          Component::uid(), _cmp_name_getter<Component>());
        return *this;
    }

    template <typename Relation>
    auto unregister_relation_type() -> auto& {
        _do_unr_stg_type<true>(Relation::uid(), _cmp_name_getter<Relation>());
        return *this;
    }

    template <typename Component>
    auto knows_component_type() const -> bool {
        return _does_know_stg_type<false>(Component::uid());
    }

    template <typename Relation>
    auto knows_relation_type() const -> bool {
        return _does_know_stg_type<true>(Relation::uid());
    }

    template <typename Component>
    auto component_storage_caps() const -> storage_caps {
        return _get_stg_type_caps<false>(
          Component::uid(), _cmp_name_getter<Component>());
    }

    template <typename Relation>
    auto relation_storage_caps() const -> storage_caps {
        return _get_stg_type_caps<true>(
          Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <typename Component>
    auto component_storage_can(const storage_cap_bit cap) const -> bool {
        return _get_stg_type_caps<false>(
                 Component::uid(), _cmp_name_getter<Component>())
          .has(cap);
    }

    template <typename Relation>
    auto relation_storage_can(const storage_cap_bit cap) const -> bool {
        return _get_stg_type_caps<true>(
                 Relation::uid(), _cmp_name_getter<Relation>())
          .has(cap);
    }

    void forget(entity_param ent);

    template <typename Component>
    auto has(entity_param ent) -> bool {
        return _does_have_c(
          ent, Component::uid(), _cmp_name_getter<Component>());
    }

    template <typename Relation>
    auto has(entity_param subject, entity_param object) -> bool {
        return _does_have_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <typename... Components>
    auto has_all(entity_param ent) -> bool {
        return _count_true(_does_have_c(
                 ent, Components::uid(), _cmp_name_getter<Components>())...) ==
               (sizeof...(Components));
    }

    template <typename Relation>
    auto is(entity_param object, entity_param subject) -> bool {
        return _does_have_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <typename Component>
    auto hidden(entity_param ent) -> bool {
        return _is_hidn(ent, Component::uid(), _cmp_name_getter<Component>());
    }

    template <typename... Components>
    auto all_hidden(entity_param ent) -> bool {
        return _count_true(_is_hidn(
                 ent, Components::uid(), _cmp_name_getter<Components>())...) ==
               (sizeof...(Components));
    }

    template <typename... Components>
    auto show(entity_param ent) -> auto& {
        (..., _do_show(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <typename... Components>
    auto hide(entity_param ent) -> auto& {
        (..., _do_hide(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <typename... Components>
    auto add(entity_param ent, Components&&... components) -> auto& {
        (..., _do_add_c(ent, std::move(components)));
        return *this;
    }

    template <typename Component>
    auto add(entity_param ent, std::type_identity<Component> = {})
      -> manipulator<Component>
        requires(Component::is_component())
    {
        return {_do_add_c(ent, Component{}), false};
    }

    template <typename Relation>
    auto add(entity_param subject, entity_param object, Relation&& rel)
      -> manipulator<Relation>
        requires(Relation::is_relation())
    {
        return {_do_add_r(subject, object, std::forward<Relation>(rel)), false};
    }

    template <typename Relation>
    auto add(entity_param subject, entity_param object) -> bool
        requires(Relation::is_relation())
    {
        return _do_add_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <typename Component>
    auto copy(entity_param from, entity_param to) -> manipulator<Component>
        requires(Component::is_component())
    {
        return {
          static_cast<Component*>(
            _do_cpy(from, to, Component::uid(), _cmp_name_getter<Component>())),
          false};
    }

    template <typename... Components>
    auto copy(entity_param from, entity_param to) -> basic_manager&
        requires(sizeof...(Components) > 1)
    {
        (...,
         _do_cpy(from, to, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <typename... Components>
    auto swap(entity_param e1, entity_param e2) -> auto& {
        (...,
         _do_swp(e1, e2, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <typename... Components>
    auto remove(entity_param ent) -> auto& {
        (...,
         _do_rem_c(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <typename Relation>
    auto remove_relation(entity_param subject, entity_param object) -> auto& {
        _do_rem_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
        return *this;
    }

    template <typename T, typename Component>
    auto get(T Component::*const mvp, entity_param ent, T res = T()) -> T {
        return _do_get_c(mvp, ent, res);
    }

    template <typename Component>
    auto for_single(
      entity_param ent,
      const callable_ref<void(entity_param, manipulator<const Component>&)>&
        func) -> auto& {
        _call_for_single_c<Component>(ent, func);
        return *this;
    }

    template <typename Component>
    auto for_single(
      const callable_ref<void(entity_param, manipulator<Component>&)>& func,
      entity_param ent) -> auto& {
        _call_for_single_c<Component>(func, ent);
        return *this;
    }

    template <typename Component>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<const Component>&)>&
        func) -> auto& {
        _call_for_each_c<Component>(func);
        return *this;
    }

    template <typename Component>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<Component>&)>& func)
      -> auto& {
        _call_for_each_c<Component>(func);
        return *this;
    }

    template <typename Relation>
    auto for_each(const callable_ref<void(entity_param, entity_param)>& func)
      -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <typename Relation>
    auto for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>& func)
      -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <typename Relation>
    auto for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>& func)
      -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <typename... Components>
    auto for_each_opt(
      const callable_ref<void(entity_param, manipulator<Components>&...)>& func)
      -> auto& {
        _call_for_each_c_m_p<Components...>(func);
        return *this;
    }

    template <typename... Components, typename Func>
    auto for_each_with_opt(const Func& func) -> auto& {
        callable_ref<void(entity_param, manipulator<Components> & ...)> wrap(
          construct_from, func);
        return for_each_opt<Components...>(wrap);
    }

    template <typename... Components>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<Components>&...)>& func)
      -> basic_manager&
        requires(sizeof...(Components) > 1)
    {
        _call_for_each_c_m_r<Components...>(func);
        return *this;
    }

    template <typename... Components, typename Func>
    auto for_each_with(const Func& func) -> auto& {
        return for_each<Components...>(
          callable_ref<void(entity_param, manipulator<Components> & ...)>{
            construct_from, func});
    }

    template <typename... Components>
    auto select()
      -> component_relation<Entity, mp_list<mp_list<Components...>>> {
        return {*this};
    }

private:
    using _base_cmp_storage_t = base_component_storage<Entity>;
    using _base_cmp_storage_ptr_t = std::unique_ptr<_base_cmp_storage_t>;

    component_uid_map<_base_cmp_storage_ptr_t> _cmp_storages{};

    auto _get_storages(std::false_type) noexcept -> auto& {
        return _cmp_storages;
    }

    auto _get_storages(std::false_type) const noexcept -> auto& {
        return _cmp_storages;
    }

    using _base_rel_storage_t = base_relation_storage<Entity>;
    using _base_rel_storage_ptr_t = std::unique_ptr<_base_rel_storage_t>;

    component_uid_map<_base_rel_storage_ptr_t> _rel_storages{};

    auto _get_storages(std::true_type) noexcept -> auto& {
        return _rel_storages;
    }

    auto _get_storages(std::true_type) const noexcept -> auto& {
        return _rel_storages;
    }

    template <bool IsR>
    auto _get_storages() noexcept -> auto& {
        return _get_storages(std::integral_constant<bool, IsR>());
    }

    template <bool IsR>
    auto _get_storages() const noexcept -> auto& {
        return _get_storages(std::integral_constant<bool, IsR>());
    }

    template <typename C>
    using _bare_t = std::remove_const_t<std::remove_reference_t<C>>;

    static constexpr auto _count_true() -> unsigned {
        return 0U;
    }

    template <typename T, typename... P>
    static constexpr auto _count_true(T v, P... p) -> unsigned {
        return (bool(v) ? 1U : 0U) + _count_true(p...);
    }

    template <typename C>
    static auto _cmp_name_getter() -> std::string (*)() {
        return &type_name<C>;
    }

    template <typename Data, bool IsR>
    auto _find_storage() -> storage<Entity, Data, IsR>&;

    template <typename C>
    auto _find_cmp_storage() -> auto& {
        return _find_storage<C, false>();
    }

    template <typename R>
    auto _find_rel_storage() -> auto& {
        return _find_storage<R, true>();
    }

    template <bool IsRelation>
    void _do_reg_stg_type(
      std::unique_ptr<base_storage<Entity, IsRelation>>&& strg,
      component_uid_t cid,
      std::string (*get_name)()) {
        assert(bool(strg));

        if(!_get_storages<IsRelation>().emplace(cid, std::move(strg))) {
            mgr_handle_cmp_is_reg(get_name());
        }
    }

    template <bool IsRelation>
    void _do_unr_stg_type(component_uid_t cid, std::string (*get_name)()) {
        if(_get_storages<IsRelation>().erase(cid) != 1) {
            mgr_handle_cmp_not_reg(get_name());
        }
    }

    template <bool IsRelation>
    auto _does_know_stg_type(component_uid_t cid) const -> bool {
        return _get_storages<IsRelation>().find(cid).is_valid();
    }

    template <bool IsR, typename Result, typename Func>
    auto _apply_on_base_stg(
      Result,
      const Func&,
      component_uid_t,
      std::string (*)()) const -> Result;

    template <typename D, bool IsR, typename Result, typename Func>
    auto _apply_on_stg(Result, const Func&) const -> Result;

    template <bool IsR>
    auto _get_stg_type_caps(component_uid_t, std::string (*)()) const
      -> storage_caps;

    auto _does_have_c(entity_param, component_uid_t, std::string (*)()) -> bool;

    auto _does_have_r(
      entity_param,
      entity_param,
      component_uid_t,
      std::string (*)()) -> bool;

    auto _is_hidn(entity_param, component_uid_t, std::string (*)()) -> bool;

    auto _do_show(entity_param, component_uid_t, std::string (*)()) -> bool;

    auto _do_hide(entity_param, component_uid_t, std::string (*)()) -> bool;

    template <typename Component>
    auto _do_add_c(entity_param, Component&& component) -> Component*;

    template <typename Relation>
    auto _do_add_r(entity_param, entity_param, Relation&& relation)
      -> Relation*;

    auto _do_add_r(
      entity_param,
      entity_param,
      component_uid_t,
      std::string (*)()) -> bool;

    auto _do_cpy(
      entity_param f,
      entity_param t,
      component_uid_t,
      std::string (*)()) -> void*;

    auto _do_swp(
      entity_param f,
      entity_param t,
      component_uid_t,
      std::string (*)()) -> bool;

    auto _do_rem_c(entity_param, component_uid_t, std::string (*)()) -> bool;

    auto _do_rem_r(
      entity_param,
      entity_param,
      component_uid_t,
      std::string (*)()) -> bool;

    template <typename C, typename Func>
    auto _call_for_single_c(entity_param, const Func&) -> bool;

    template <typename C, typename Func>
    void _call_for_each_c(const Func&);

    template <typename R, typename Func>
    void _call_for_each_r(const Func&);

    template <typename... C, typename Func>
    void _call_for_each_c_m_p(const Func&);

    template <typename... C, typename Func>
    void _call_for_each_c_m_r(const Func&);

    template <typename T, typename C>
    auto _do_get_c(T C::*const, entity_param, T) -> T;
};
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Data, bool IsR>
auto basic_manager<Entity>::_find_storage() -> storage<Entity, Data, IsR>& {

    using S = storage<Entity, Data, IsR>;
    S* pd_storage = nullptr;

    if(auto found{_get_storages<IsR>().find(Data::uid())}) {
        auto& b_storage = extract(found);
        if(b_storage) {
            pd_storage = dynamic_cast<S*>(b_storage.get());
            assert(pd_storage);
        }
    }
    if(!pd_storage) {
        std::string (*get_name)() = _cmp_name_getter<Data>();
        mgr_handle_cmp_not_reg(get_name());
        unreachable();
    }
    return *pd_storage;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation, typename Result, typename Func>
auto basic_manager<Entity>::_apply_on_base_stg(
  Result fallback,
  const Func& func,
  component_uid_t cid,
  std::string (*get_name)()) const -> Result {
    auto& storages = _get_storages<IsRelation>();

    if(auto found{storages.find(cid)}) {
        auto& bs_storage = extract(found);
        if(bs_storage) {
            return func(bs_storage);
        }
    }
    mgr_handle_cmp_not_reg(get_name());
    return fallback;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, bool IsRelation, typename Result, typename Func>
auto basic_manager<Entity>::_apply_on_stg(Result fallback, const Func& func)
  const -> Result {
    return _apply_on_base_stg<IsRelation>(
      fallback,
      [&func](auto& b_storage) -> Result {
          using S = storage<Entity, Component, IsRelation>;

          S* ct_storage = dynamic_cast<S*>(b_storage.get());
          assert(ct_storage);

          return func(ct_storage);
      },
      Component::uid(),
      _cmp_name_getter<Component>());
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation>
auto basic_manager<Entity>::_get_stg_type_caps(
  component_uid_t cid,
  std::string (*get_name)()) const -> storage_caps {
    return _apply_on_base_stg<IsRelation>(
      storage_caps(),
      [](auto& b_storage) -> storage_caps { return b_storage->capabilities(); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_does_have_c(
  entity_param_t<Entity> ent,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&ent](auto& b_storage) -> bool { return b_storage->has(ent); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_does_have_r(
  entity_param_t<Entity> subject,
  entity_param_t<Entity> object,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<true>(
      false,
      [&subject, &object](auto& b_storage) -> bool {
          return b_storage->has(subject, object);
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_is_hidn(
  entity_param_t<Entity> ent,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&ent](auto& b_storage) -> bool { return b_storage->hidden(ent); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_show(
  entity_param_t<Entity> ent,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&ent](auto& b_storage) -> bool { return b_storage->show(ent); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_hide(
  entity_param_t<Entity> ent,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&ent](auto& b_storage) -> bool { return b_storage->hide(ent); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component>
auto basic_manager<Entity>::_do_add_c(
  entity_param_t<Entity> ent,
  Component&& component) -> Component* {
    return _apply_on_stg<Component, false>(
      static_cast<Component*>(nullptr), [&ent, &component](auto& c_storage) {
          return c_storage->store(ent, std::forward<Component>(component));
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Relation>
auto basic_manager<Entity>::_do_add_r(
  entity_param subj,
  entity_param obj,
  Relation&& relation) -> Relation* {
    return _apply_on_stg<Relation, true>(
      static_cast<Relation*>(nullptr),
      [&subj, &obj, &relation](auto& r_storage) {
          return r_storage->store(subj, obj, std::forward<Relation>(relation));
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_add_r(
  entity_param subject,
  entity_param object,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<true>(
      false,
      [&subject, &object](auto& b_storage) -> bool {
          return b_storage->store(subject, object);
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_cpy(
  entity_param_t<Entity> from,
  entity_param_t<Entity> to,
  component_uid_t cid,
  std::string (*get_name)()) -> void* {
    return _apply_on_base_stg<false>(
      static_cast<void*>(nullptr),
      [&from, &to](auto& b_storage) -> void* {
          return b_storage->copy(from, to);
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_swp(
  entity_param_t<Entity> e1,
  entity_param_t<Entity> e2,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&e1, &e2](auto& b_storage) -> bool {
          b_storage->swap(e1, e2);
          return true;
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_rem_c(
  entity_param_t<Entity> ent,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<false>(
      false,
      [&ent](auto& b_storage) -> bool { return b_storage->remove(ent); },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_rem_r(
  entity_param_t<Entity> subj,
  entity_param_t<Entity> obj,
  component_uid_t cid,
  std::string (*get_name)()) -> bool {
    return _apply_on_base_stg<true>(
      false,
      [&subj, &obj](auto& b_storage) -> bool {
          return b_storage->remove(subj, obj);
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename T, typename C>
auto basic_manager<Entity>::_do_get_c(
  T C::*const mvp,
  entity_param_t<Entity> ent,
  T res) -> T {
    assert(mvp);

    using MC = manipulator<const C>;

    const auto getter = [mvp, &res](entity_param_t<Entity>, MC& cmp) {
        res = cmp.read().*mvp;
    };

    _call_for_single_c<C>(
      ent,
      callable_ref<void(entity_param, manipulator<const C>&)>{
        construct_from, getter});
    return res;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, typename Func>
auto basic_manager<Entity>::_call_for_single_c(
  entity_param_t<Entity> ent,
  const Func& func) -> bool {
    return _apply_on_stg<std::remove_const_t<Component>, false>(
      false, [&func, &ent](auto& c_storage) -> bool {
          c_storage->for_single(func, ent);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, typename Func>
void basic_manager<Entity>::_call_for_each_c(const Func& func) {
    _apply_on_stg<std::remove_const_t<Component>, false>(
      false, [&func](auto& c_storage) -> bool {
          c_storage->for_each(func);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Relation, typename Func>
void basic_manager<Entity>::_call_for_each_r(const Func& func) {
    _apply_on_stg<std::remove_const_t<Relation>, true>(
      false, [&func](auto& c_storage) -> bool {
          c_storage->for_each(func);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity, typename C>
class _manager_for_each_c_m_base {
private:
    component_storage<Entity, std::remove_const_t<C>>& _storage;

protected:
    component_storage_iterator<Entity> _iter;
    Entity _curr;

    _manager_for_each_c_m_base(
      component_storage<Entity, std::remove_const_t<C>>& strg)
      : _storage(strg)
      , _iter(_storage.new_iterator())
      , _curr(_iter.done() ? Entity() : _iter.current()) {
        assert(std::is_const<C>::value || _storage.capabilities().can_modify());
    }

    ~_manager_for_each_c_m_base() {
        _storage.delete_iterator(std::move(_iter));
    }

    auto _done() -> bool {
        return _iter.done();
    }

    auto _current() -> entity_param_t<Entity> {
        return _curr;
    }

    void _apply(
      const callable_ref<void(entity_param_t<Entity>, manipulator<C>&)>& func) {
        _storage.for_single(func, _iter);
    }

    void _store(entity_param_t<Entity> e, std::remove_const_t<C>&& c) {
        _storage.store(_iter, e, std::move(c));
    }

public:
    _manager_for_each_c_m_base(_manager_for_each_c_m_base&&) = delete;
    _manager_for_each_c_m_base(const _manager_for_each_c_m_base&) = delete;
    auto operator=(_manager_for_each_c_m_base&&) = delete;
    auto operator=(const _manager_for_each_c_m_base&) = delete;
};
//------------------------------------------------------------------------------
template <typename Entity, typename C>
class _manager_for_each_c_m_p_base
  : public _manager_for_each_c_m_base<Entity, C> {
protected:
    _manager_for_each_c_m_p_base(
      component_storage<Entity, std::remove_const_t<C>>& strg)
      : _manager_for_each_c_m_base<Entity, C>(strg) {}

    using _manager_for_each_c_m_base<Entity, C>::_iter;
    using _manager_for_each_c_m_base<Entity, C>::_curr;
    using _manager_for_each_c_m_base<Entity, C>::_done;
    using _manager_for_each_c_m_base<Entity, C>::_current;

    void _next_if(entity_param_t<Entity> m) {
        if(!_done()) {
            if(_current() == m) {
                _iter.next();
                if(!_done()) {
                    _curr = _iter.current();
                }
            }
        }
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename LL, typename LR>
class _manager_for_each_c_m_p_unit;
//------------------------------------------------------------------------------
template <typename Entity, typename... CL, typename C>
class _manager_for_each_c_m_p_unit<Entity, mp_list<CL...>, mp_list<C>>
  : public _manager_for_each_c_m_p_base<Entity, C> {
private:
    callable_ref<
      void(entity_param_t<Entity>, manipulator<CL>&..., manipulator<C>&)>
      _func;

public:
    _manager_for_each_c_m_p_unit(
      callable_ref<
        void(entity_param_t<Entity>, manipulator<CL>&..., manipulator<C>&)> func,
      component_storage<Entity, std::remove_const_t<C>>& s)
      : _manager_for_each_c_m_p_base<Entity, C>{s}
      , _func{std::move(func)} {}

    auto done() -> bool {
        return this->_done();
    }

    void next_if_min(entity_param_t<Entity> m) {
        this->_next_if(m);
    }

    auto min_entity() -> Entity {
        return this->_current();
    }

    void apply(entity_param_t<Entity> m, manipulator<CL>&... clm) {
        if(this->_done() || (m < this->_current())) {
            std::remove_const_t<C> cadd;
            concrete_manipulator<C> cman(nullptr, cadd, false);
            _func(m, clm..., cman);
            if(cman.add_requested()) {
                this->_store(m, std::move(cadd));
            }
        } else {
            assert(m == this->_current());
            const auto hlpr = [&clm..., this](
                                entity_param_t<Entity> e, manipulator<C>& cm) {
                _func(e, clm..., cm);
            };
            this->_apply({construct_from, hlpr});
        }
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename... CL, typename C, typename... CR>
class _manager_for_each_c_m_p_unit<Entity, mp_list<CL...>, mp_list<C, CR...>>
  : public _manager_for_each_c_m_p_base<Entity, C> {
private:
    _manager_for_each_c_m_p_unit<Entity, mp_list<CL..., C>, mp_list<CR...>>
      _rest;

public:
    _manager_for_each_c_m_p_unit(
      const callable_ref<void(
        entity_param_t<Entity>,
        manipulator<CL>&...,
        manipulator<C>&,
        manipulator<CR>&...)>& func,
      component_storage<Entity, std::remove_const_t<C>>& s,
      component_storage<Entity, std::remove_const_t<CR>>&... r)
      : _manager_for_each_c_m_p_base<Entity, C>(s)
      , _rest(func, r...) {}

    auto done() -> bool {
        return this->_done() && _rest.done();
    }

    void next_if_min(entity_param_t<Entity> m) {
        _rest.next_if_min(m);
        this->_next_if(m);
    }

    auto min_entity() -> Entity {
        if(_rest.done()) {
            assert(!this->_done());
            return this->_current();
        }

        Entity m = _rest.min_entity();
        if(this->_done()) {
            return m;
        }
        Entity c = this->_current();
        return (m < c) ? m : c;
    }

    void next() {
        next_if_min(min_entity());
    }

    void apply(
      [[maybe_unused]] entity_param_t<Entity> m,
      manipulator<CL>&... clm) {
        if(this->_done() || (m < this->_current())) {
            std::remove_const_t<C> cadd;
            concrete_manipulator<C> cman(nullptr, cadd, false);
            _rest.apply(m, clm..., cman);
            if(cman.add_requested()) {
                this->_store(m, std::move(cadd));
            }
        } else {
            assert(m == this->_current());
            const auto hlpr = [&clm..., this](
                                entity_param_t<Entity> e, manipulator<C>& cm) {
                _rest.apply(e, clm..., cm);
            };
            this->_apply({construct_from, hlpr});
        }
    }

    void apply() {
        static_assert(sizeof...(CL) == 0);
        assert(!done());

        apply(min_entity());
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename... C>
using _manager_for_each_c_m_p_helper =
  _manager_for_each_c_m_p_unit<Entity, mp_list<>, mp_list<C...>>;
//------------------------------------------------------------------------------
template <typename Entity>
template <typename... Component, typename Func>
void basic_manager<Entity>::_call_for_each_c_m_p(const Func& func) {
    _manager_for_each_c_m_p_helper<Entity, Component...> hlp(
      func, _find_cmp_storage<_bare_t<Component>>()...);
    while(!hlp.done()) {
        hlp.apply();
        hlp.next();
    }
}
//------------------------------------------------------------------------------
template <typename Entity, typename C>
class _manager_for_each_c_m_r_base
  : public _manager_for_each_c_m_base<Entity, C> {
protected:
    _manager_for_each_c_m_r_base(
      component_storage<Entity, std::remove_const_t<C>>& storage)
      : _manager_for_each_c_m_base<Entity, C>(storage) {}

    using _manager_for_each_c_m_base<Entity, C>::_iter;
    using _manager_for_each_c_m_base<Entity, C>::_curr;
    using _manager_for_each_c_m_base<Entity, C>::_done;
    using _manager_for_each_c_m_base<Entity, C>::_current;

    auto _next() -> bool {
        if(_current() >= _iter.current()) {
            _iter.next();
        }
        if(!_iter.done()) {
            _curr = _iter.current();
            return true;
        }
        return false;
    }

    auto _find(entity_param_t<Entity> e) -> bool {
        if(_iter.find(e)) {
            _curr = _iter.current();
            return true;
        }
        return false;
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename LL, typename LR>
class _manager_for_each_c_m_r_unit;
//------------------------------------------------------------------------------
template <typename Entity, typename... CL, typename C>
class _manager_for_each_c_m_r_unit<Entity, mp_list<CL...>, mp_list<C>>
  : public _manager_for_each_c_m_r_base<Entity, C> {
private:
    callable_ref<
      void(entity_param_t<Entity>, manipulator<CL>&..., manipulator<C>&)>
      _func;

public:
    _manager_for_each_c_m_r_unit(
      const callable_ref<
        void(entity_param_t<Entity>, manipulator<CL>&..., manipulator<C>&)> func,
      component_storage<Entity, std::remove_const_t<C>>& s)
      : _manager_for_each_c_m_r_base<Entity, C>{s}
      , _func{std::move(func)} {}

    auto done() -> bool {
        return this->_done();
    }

    auto sync_to(entity_param_t<Entity> m) -> bool {
        return this->_find(m);
    }

    auto max_entity() -> Entity {
        return this->_current();
    }

    auto next() -> bool {
        return this->_next();
    }

    void apply(
      [[maybe_unused]] entity_param_t<Entity> m,
      manipulator<CL>&... clm) {
        assert(m == this->_current());
        const auto hlpr = [&clm..., this](
                            entity_param_t<Entity> e, manipulator<C>& cm) {
            _func(e, clm..., cm);
        };
        this->_apply({construct_from, hlpr});
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename... CL, typename C, typename... CR>
class _manager_for_each_c_m_r_unit<Entity, mp_list<CL...>, mp_list<C, CR...>>
  : public _manager_for_each_c_m_r_base<Entity, C> {
private:
    _manager_for_each_c_m_r_unit<Entity, mp_list<CL..., C>, mp_list<CR...>>
      _rest;

public:
    _manager_for_each_c_m_r_unit(
      const callable_ref<void(
        entity_param_t<Entity>,
        manipulator<CL>&...,
        manipulator<C>&,
        manipulator<CR>&...)>& func,
      component_storage<Entity, std::remove_const_t<C>>& s,
      component_storage<Entity, std::remove_const_t<CR>>&... r)
      : _manager_for_each_c_m_r_base<Entity, C>(s)
      , _rest(func, r...) {}

    auto done() -> bool {
        return this->_done() || _rest.done();
    }

    auto sync_to(entity_param_t<Entity> m) -> bool {
        return _rest.sync_to(m) && this->_find(m);
    }

    auto max_entity() -> Entity {
        Entity m = _rest.max_entity();
        Entity c = this->_current();
        return (m > c) ? m : c;
    }

    auto sync() -> bool {
        static_assert(sizeof...(CL) == 0);
        return sync_to(max_entity());
    }

    auto next() -> bool {
        return _rest.next() && this->_next();
    }

    void apply(
      [[maybe_unused]] entity_param_t<Entity> m,
      manipulator<CL>&... clm) {
        assert(m == this->_current());
        const auto hlpr = [&clm..., this](
                            entity_param_t<Entity> e, manipulator<C>& cm) {
            _rest.apply(e, clm..., cm);
        };
        this->_apply({construct_from, hlpr});
    }

    void apply() {
        static_assert(sizeof...(CL) == 0);
        assert(!done());

        apply(max_entity());
    }
};
//------------------------------------------------------------------------------
template <typename Entity, typename... C>
using _manager_for_each_c_m_r_helper =
  _manager_for_each_c_m_r_unit<Entity, mp_list<>, mp_list<C...>>;
//------------------------------------------------------------------------------
template <typename Entity>
template <typename... Component, typename Func>
void basic_manager<Entity>::_call_for_each_c_m_r(const Func& func) {
    _manager_for_each_c_m_r_helper<Entity, Component...> hlp(
      func, _find_cmp_storage<_bare_t<Component>>()...);
    if(hlp.sync()) {
        while(!hlp.done()) {
            hlp.apply();
            if(!hlp.next()) {
                break;
            }
            if(!hlp.sync()) {
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------
template <typename Entity>
void basic_manager<Entity>::forget(entity_param_t<Entity> ent) {
    for(auto& storage : _cmp_storages) {
        if(storage != nullptr) {
            if(storage->caps().can_remove()) {
                storage->remove(ent);
            }
        }
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::ecs
