/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:manager;

import std;
import eagine.core.debug;
import eagine.core.types;
import eagine.core.string;
import eagine.core.valid_if;
import eagine.core.utility;
import :entity_traits;
import :manipulator;
import :component;
import :storage;

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

/// @brief Class that can be used to do Cartesian products of various component types.
/// @see ecs
export template <typename Entity, typename... PL>
class component_relation<Entity, mp_list<PL...>> {

public:
    component_relation(basic_manager<Entity>& m) noexcept
      : _m{m} {}

    /// @brief Extend this relation with a cross product of additional components.
    template <component_data... Components>
    auto cross() noexcept
      -> component_relation<Entity, mp_list<PL..., mp_list<Components...>>> {
        return {_m};
    }

    /// @brief Call the specified function on each combination of component objects.
    template <typename Func>
    void for_each(const Func& function) {
        _apply(_m, function, mp_list<PL...>());
    }

private:
    basic_manager<Entity>& _m;

    template <typename F, typename... C, typename... X>
    static auto _apply(
      basic_manager<Entity>& m,
      const F& func,
      const mp_list<mp_list<C...>>,
      X&&... x) {
        const auto wrap{
          [&func, &x...](entity_param_t<Entity> e, manipulator<C>&... c) {
              func(std::forward<X>(x)..., e, c...);
          }};
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
        const auto wrap{
          [&m, &func, &x...](entity_param_t<Entity> e, manipulator<C>&... c) {
              _apply(
                m, func, mp_list<L, Ls...>(), std::forward<X>(x)..., e, c...);
          }};
        m.for_each(
          callable_ref<void(entity_param_t<Entity>, manipulator<C> & ...)>{
            construct_from, wrap});
    }
};
//------------------------------------------------------------------------------
/// @brief Main class, managing entity data. Entities are represented by values of Entity.
/// @ingroup ecs
export template <typename Entity>
class basic_manager : public basic_manager_signals<Entity> {
public:
    /// @brief Preferred type to pass immutable entity identifier parameters.
    using entity_param = entity_param_t<Entity>;

    /// @brief Default constructor.
    constexpr basic_manager() noexcept = default;

    /// @brief Registers the storage to be used to store instances of Component type.
    /// @see unregister_component_type
    /// @see knows_component_type
    /// @see register_relation_storage
    template <component_data Component>
    auto register_component_type(
      shared_holder<component_storage<Entity, Component>>&& strg) -> auto& {
        _do_reg_stg_type<data_kind::component>(
          _base_cmp_storage_ptr_t(std::move(strg)),
          Component::uid(),
          _cmp_name_getter<Component>());
        return *this;
    }

    /// @brief Registers the storage to be used to store instances of Relation type.
    /// @see unregister_relation_type
    /// @see knows_relation_type
    /// @see register_component_storage
    template <relation_data Relation>
    auto register_relation_type(
      shared_holder<relation_storage<Entity, Relation>>&& strg) -> auto& {
        _do_reg_stg_type<data_kind::relation>(
          _base_rel_storage_ptr_t(std::move(strg)),
          Relation::uid(),
          _cmp_name_getter<Relation>());
        return *this;
    }

    /// @brief Registers the storage to be used to store instances of Component type.
    /// @see unregister_component_type
    /// @see knows_component_type
    /// @see register_relation_storage
    template <
      template <class, class>
      class Storage,
      component_data Component,
      typename... P>
    auto register_component_storage(P&&... p) -> auto& {
        register_component_type<Component>(
          {hold<Storage<Entity, Component>>, std::forward<P>(p)...});
        return *this;
    }

    template <template <class, class> class Storage, component_data... Components>
    auto register_component_storages() -> auto& {
        (void)(..., register_component_storage<Storage, Components>());
        return *this;
    }

    /// @brief Registers the storage to be used to store instances of Relation type.
    /// @see unregister_relation_type
    /// @see knows_relation_type
    /// @see register_component_storage
    template <
      template <class, class>
      class Storage,
      relation_data Relation,
      typename... P>
    auto register_relation_storage(P&&... p) -> auto& {
        register_relation_type<Relation>(
          {hold<Storage<Entity, Relation>>, std::forward<P>(p)...});
        return *this;
    }

    template <template <class, class> class Storage, relation_data... Relations>
    auto register_relation_storages() -> auto& {
        (void)(..., register_relation_storage<Storage, Relations>());
        return *this;
    }

    /// @brief Unregisters the specified Component type.
    /// @see register_component_storage
    /// @see knows_component_type
    /// @see unregister_relation_type
    template <component_data Component>
    auto unregister_component_type() -> auto& {
        _do_unr_stg_type<data_kind::component>(
          Component::uid(), _cmp_name_getter<Component>());
        return *this;
    }

    /// @brief Unregisters the specified Relation type.
    /// @see register_relation_storage
    /// @see knows_relation_type
    /// @see unregister_component_type
    template <relation_data Relation>
    auto unregister_relation_type() -> auto& {
        _do_unr_stg_type<data_kind::relation>(
          Relation::uid(), _cmp_name_getter<Relation>());
        return *this;
    }

    /// @brief Indicates if the specified Component type can be used with this manager.
    /// @see register_component_storage
    /// @see register_relation_storage
    /// @see component_storage_caps
    /// @see knows_relation_type
    template <component_data Component>
    auto knows_component_type() const -> bool {
        return _does_know_stg_type<data_kind::component>(Component::uid());
    }

    /// @brief Indicates if the specified Relation type can be used with this manager.
    /// @see register_relation_storage
    /// @see register_component_storage
    /// @see relation_storage_caps
    /// @see knows_component_type
    template <relation_data Relation>
    [[nodiscard]] auto knows_relation_type() const -> bool {
        return _does_know_stg_type<data_kind::relation>(Relation::uid());
    }

    /// @brief Returns an object specifying Component storage capabilities.
    /// @see component_storage_can
    /// @see register_component_storage
    /// @see knows_component_type
    template <component_data Component>
    [[nodiscard]] auto component_storage_caps() const -> storage_caps {
        return _get_stg_type_caps<data_kind::component>(
          Component::uid(), _cmp_name_getter<Component>());
    }

    /// @brief Returns an object specifying Relation storage capabilities.
    /// @see relation_storage_can
    /// @see register_relation_storage
    /// @see knows_relation_type
    template <relation_data Relation>
    [[nodiscard]] auto relation_storage_caps() const -> storage_caps {
        return _get_stg_type_caps<true>(
          Relation::uid(), _cmp_name_getter<Relation>());
    }

    /// @brief Indicates if the storage object for Component has specified capability.
    /// @see component_storage_caps
    /// @see register_component_storage
    /// @see knows_component_type
    template <component_data Component>
    [[nodiscard]] auto component_storage_can(const storage_cap_bit cap) const
      -> bool {
        return _get_stg_type_caps<false>(
                 Component::uid(), _cmp_name_getter<Component>())
          .has(cap);
    }

    /// @brief Indicates if the storage object for Relation has specified capability.
    /// @see relation_storage_caps
    /// @see register_relation_storage
    /// @see knows_relation_type
    template <relation_data Relation>
    [[nodiscard]] auto relation_storage_can(const storage_cap_bit cap) const
      -> bool {
        return _get_stg_type_caps<true>(
                 Relation::uid(), _cmp_name_getter<Relation>())
          .has(cap);
    }

    /// @brief Indicates if the manager has any knowledge about the specified entity.
    /// @see has
    /// @see has_all
    /// @see forget
    /// @see spawn
    auto knows(entity_param ent) noexcept -> bool;

    /// @brief Creates a new entity that is not yet known to this manager
    /// @see knows
    /// @see forget
    auto spawn() noexcept -> Entity;

    /// @brief Removes all information, including components, about the
    /// specified entity.
    /// @see has
    /// @see has_all
    /// @see knows
    void forget(entity_param ent);

    /// @brief Indicates if the specified entity has the specified Component.
    /// @see has_all
    /// @see knows
    /// @see forget
    template <component_data Component>
    [[nodiscard]] auto has(entity_param ent) noexcept -> bool {
        return _does_have_c(
          ent, Component::uid(), _cmp_name_getter<Component>());
    }

    /// @brief Indicates if the specified subject has the specified Relation with object.
    /// @see has_all
    /// @see knows
    /// @see forget
    template <relation_data Relation>
    [[nodiscard]] auto has(entity_param subject, entity_param object) noexcept
      -> bool {
        return _does_have_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    /// @brief Indicates if the specified entity has all the specified Components.
    /// @see has
    /// @see forget
    template <component_data... Components>
    [[nodiscard]] auto has_all(entity_param ent) -> bool {
        return (
          ... and
          _does_have_c(ent, Components::uid(), _cmp_name_getter<Components>()));
    }

    template <relation_data Relation>
    [[nodiscard]] auto is(entity_param object, entity_param subject) noexcept
      -> bool {
        return _does_have_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <component_data Component>
    [[nodiscard]] auto is_hidden(entity_param ent) noexcept -> bool {
        return _is_hidn(ent, Component::uid(), _cmp_name_getter<Component>());
    }

    template <component_data... Components>
    [[nodiscard]] auto are_hidden(entity_param ent) noexcept -> bool {
        return (
          ... and
          _is_hidn(ent, Components::uid(), _cmp_name_getter<Components>()));
    }

    template <component_data... Components>
    auto show(entity_param ent) -> auto& {
        (..., _do_show(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <component_data... Components>
    auto hide(entity_param ent) -> auto& {
        (..., _do_hide(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <component_data... Components>
    auto add(entity_param ent, Components&&... components) -> auto& {
        (..., _do_add_c(ent, std::move(components)));
        return *this;
    }

    template <component_data Component>
    auto ensure(entity_param ent, std::type_identity<Component> = {})
      -> manipulator<Component> {
        return {_do_add_c(ent, Component{}), false};
    }

    template <relation_data Relation>
    auto add(entity_param subject, entity_param object, Relation&& rel)
      -> manipulator<Relation> {
        return {_do_add_r(subject, object, std::forward<Relation>(rel)), false};
    }

    template <relation_data Relation>
    auto ensure(
      entity_param subject,
      entity_param object,
      std::type_identity<Relation> = {}) -> bool {
        return _do_add_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
    }

    template <component_data Component>
    auto copy(entity_param from, entity_param to) -> manipulator<Component> {
        return {
          _do_cpy<Component>(
            from, to, Component::uid(), _cmp_name_getter<Component>()),
          false};
    }

    template <component_data... Components>
    auto copy(entity_param from, entity_param to) -> basic_manager&
        requires(sizeof...(Components) > 1)
    {
        (...,
         _do_cpy<Components>(
           from, to, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <component_data... Components>
    auto exchange(entity_param e1, entity_param e2) -> auto& {
        (...,
         _do_xchg(e1, e2, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <component_data... Components>
    auto remove(entity_param ent) -> auto& {
        (...,
         _do_rem_c(ent, Components::uid(), _cmp_name_getter<Components>()));
        return *this;
    }

    template <relation_data Relation>
    auto remove_relation(entity_param subject, entity_param object) -> auto& {
        _do_rem_r(
          subject, object, Relation::uid(), _cmp_name_getter<Relation>());
        return *this;
    }

    template <typename T, component_data Component>
    [[nodiscard]] auto get(
      T Component::*const mvp,
      entity_param ent,
      T res = T()) -> T {
        return _do_get_c(mvp, ent, res);
    }

    template <component_data Component>
    auto for_single(
      entity_param ent,
      const callable_ref<void(entity_param, manipulator<const Component>&)>&
        func) -> auto& {
        _call_for_single_c<Component>(ent, func);
        return *this;
    }

    template <component_data Component, typename Function>
    auto read_single(entity_param ent, Function&& function) -> auto& {
        return for_single(
          ent,
          callable_ref<void(entity_param, manipulator<const Component>&)>{
            construct_from, std::forward<Function>(function)});
    }

    template <component_data... Component, typename Function>
    auto read(entity_param ent, Function&& function) -> auto& {
        _call_for_single_c_p(
          mp_list<std::add_const_t<Component>...>{}, function, ent);
        return *this;
    }

    template <component_data Component>
    auto for_single(
      entity_param ent,
      const callable_ref<void(entity_param, manipulator<Component>&)>& func)
      -> auto& {
        _call_for_single_c<Component>(ent, func);
        return *this;
    }

    template <component_data Component, typename Function>
    auto write_single(entity_param ent, Function&& function) -> auto& {
        return for_single(
          ent,
          callable_ref<void(entity_param, manipulator<Component>&)>{
            construct_from, std::forward<Function>(function)});
    }

    template <component_data... Component, typename Function>
    auto write(entity_param ent, Function&& function) -> auto& {
        _call_for_single_c_p(
          mp_list<std::remove_const_t<Component>...>{}, function, ent);
        return *this;
    }

    template <component_data Component>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<const Component>&)>&
        func) -> auto& {
        _call_for_each_c<Component>(func);
        return *this;
    }

    template <component_data Component, typename Function>
    auto read_each(Function&& function) -> auto& {
        return for_each(
          callable_ref<void(entity_param, manipulator<const Component>&)>{
            construct_from, std::forward<Function>(function)});
    }

    template <component_data Component>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<Component>&)>& func)
      -> auto& {
        _call_for_each_c<Component>(func);
        return *this;
    }

    template <component_data Component, typename Function>
    auto write_each(Function&& function) -> auto& {
        return for_each(
          callable_ref<void(entity_param, manipulator<Component>&)>{
            construct_from, std::forward<Function>(function)});
    }

    template <relation_data Relation>
    auto for_each_having(
      const callable_ref<void(entity_param, entity_param)>& func) -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <relation_data Relation>
    auto for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)>& func)
      -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <relation_data Relation>
    auto for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)>& func)
      -> auto& {
        _call_for_each_r<Relation>(func);
        return *this;
    }

    template <component_data... Components>
    auto for_each_opt(
      const callable_ref<void(entity_param, manipulator<Components>&...)>& func)
      -> auto& {
        _call_for_each_c_m_p<Components...>(func);
        return *this;
    }

    template <component_data... Components, typename Func>
    auto for_each_with_opt(const Func& func) -> auto& {
        callable_ref<void(entity_param, manipulator<Components> & ...)> wrap(
          construct_from, func);
        return for_each_opt<Components...>(wrap);
    }

    template <component_data... Components>
    auto for_each(
      const callable_ref<void(entity_param, manipulator<Components>&...)>& func)
      -> basic_manager&
        requires(sizeof...(Components) > 1)
    {
        _call_for_each_c_m_r<Components...>(func);
        return *this;
    }

    template <component_data... Components, typename Func>
    auto for_each_with(const Func& func) -> auto& {
        return for_each<Components...>(
          callable_ref<void(entity_param, manipulator<Components> & ...)>{
            construct_from, func});
    }

    template <component_data... Components>
    [[nodiscard]] auto select()
      -> component_relation<Entity, mp_list<mp_list<Components...>>> {
        return {*this};
    }

    auto clear() noexcept -> basic_manager& {
        _cmp_storages.clear();
        _rel_storages.clear();
        return *this;
    }

private:
    using _base_cmp_storage_t = base_component_storage<Entity>;
    using _base_cmp_storage_ptr_t = shared_holder<_base_cmp_storage_t>;

    Entity _entity_sequence{entity_traits<Entity>::first()};

    component_uid_map<_base_cmp_storage_ptr_t> _cmp_storages{};

    auto _get_storages(
      std::integral_constant<data_kind, data_kind::component>) noexcept
      -> auto& {
        return _cmp_storages;
    }

    auto _get_storages(std::integral_constant<data_kind, data_kind::component>)
      const noexcept -> auto& {
        return _cmp_storages;
    }

    using _base_rel_storage_t = base_relation_storage<Entity>;
    using _base_rel_storage_ptr_t = shared_holder<_base_rel_storage_t>;

    component_uid_map<_base_rel_storage_ptr_t> _rel_storages{};

    auto _get_storages(
      std::integral_constant<data_kind, data_kind::relation>) noexcept
      -> auto& {
        return _rel_storages;
    }

    auto _get_storages(std::integral_constant<data_kind, data_kind::relation>)
      const noexcept -> auto& {
        return _rel_storages;
    }

    template <data_kind kind>
    auto _get_storages() noexcept -> auto& {
        return _get_storages(std::integral_constant<data_kind, kind>());
    }

    template <data_kind kind>
    auto _get_storages() const noexcept -> auto& {
        return _get_storages(std::integral_constant<data_kind, kind>());
    }

    template <typename C>
    using _bare_t = std::remove_const_t<std::remove_reference_t<C>>;

    template <typename C>
    static auto _cmp_name_getter() noexcept -> std::string (*)() noexcept {
        return &type_name<C>;
    }

    template <typename Data, data_kind kind>
    auto _find_storage() noexcept -> storage<Entity, Data, kind>&;

    template <typename C>
    auto _find_cmp_storage() noexcept -> auto& {
        return _find_storage<C, data_kind::component>();
    }

    template <typename R>
    auto _find_rel_storage() noexcept -> auto& {
        return _find_storage<R, data_kind::relation>();
    }

    template <data_kind kind>
    void _do_reg_stg_type(
      shared_holder<base_storage<Entity, kind>>&& strg,
      identifier_t cid,
      std::string (*get_name)() noexcept) {
        assert(bool(strg));

        if(not _get_storages<kind>().emplace(cid, std::move(strg))) {
            mgr_handle_cmp_is_reg(get_name());
        }
    }

    template <data_kind kind>
    void _do_unr_stg_type(identifier_t cid, std::string (*get_name)() noexcept) {
        if(_get_storages<kind>().erase(cid) != 1) {
            mgr_handle_cmp_not_reg(get_name());
        }
    }

    template <data_kind kind>
    auto _does_know_stg_type(identifier_t cid) const noexcept -> bool {
        return _get_storages<kind>().find(cid).has_value();
    }

    template <data_kind, typename Func>
    auto _apply_on_base_stg(
      const Func&,
      identifier_t,
      std::string (*)() noexcept) const;

    template <typename D, data_kind, typename Func>
    auto _apply_on_stg(const Func&) const;

    template <data_kind>
    auto _get_stg_type_caps(identifier_t, std::string (*)() noexcept)
      const noexcept -> storage_caps;

    auto _does_have_c(
      entity_param,
      identifier_t,
      std::string (*)() noexcept) noexcept -> bool;

    auto _does_have_r(
      entity_param,
      entity_param,
      identifier_t,
      std::string (*)() noexcept) noexcept -> bool;

    auto _is_hidn(
      entity_param,
      identifier_t,
      std::string (*)() noexcept) noexcept -> bool;

    auto _do_show(entity_param, identifier_t, std::string (*)() noexcept)
      -> bool;

    auto _do_hide(entity_param, identifier_t, std::string (*)() noexcept)
      -> bool;

    template <typename Component>
    auto _do_add_c(entity_param, Component&& component)
      -> optional_reference<Component>;

    template <typename Relation>
    auto _do_add_r(entity_param, entity_param, Relation&& relation)
      -> optional_reference<Relation>;

    auto _do_add_r(
      entity_param,
      entity_param,
      identifier_t,
      std::string (*)() noexcept) -> bool;

    template <typename Component>
    auto _do_cpy(
      entity_param f,
      entity_param t,
      identifier_t,
      std::string (*)() noexcept) -> optional_reference<Component>;

    auto _do_xchg(
      entity_param f,
      entity_param t,
      identifier_t,
      std::string (*)() noexcept) -> bool;

    auto _do_rem_c(entity_param, identifier_t, std::string (*)() noexcept)
      -> bool;

    auto _do_rem_r(
      entity_param,
      entity_param,
      identifier_t,
      std::string (*)() noexcept) -> bool;

    template <typename Func, typename... M>
    void _call_for_single_c_p(
      mp_list<>,
      const Func& func,
      entity_param_t<Entity> ent,
      manipulator<M>&... m);

    template <typename Func, typename C, typename... Cs, typename... M>
    void _call_for_single_c_p(
      mp_list<C, Cs...>,
      const Func& func,
      entity_param_t<Entity> ent,
      manipulator<M>&... m);

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
template <typename Data, data_kind kind>
auto basic_manager<Entity>::_find_storage() noexcept
  -> storage<Entity, Data, kind>& {

    using S = storage<Entity, Data, kind>;
    auto found{_get_storages<kind>()
                 .find(Data::uid())
                 .and_then([](auto& b_storage) -> optional_reference<S> {
                     return dynamic_cast<S*>(b_storage.get());
                 })};

    if(not found) {
        std::string (*get_name)() noexcept = _cmp_name_getter<Data>();
        mgr_handle_cmp_not_reg(get_name());
        unreachable();
    }
    return *found;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <data_kind kind, typename Func>
auto basic_manager<Entity>::_apply_on_base_stg(
  const Func& func,
  identifier_t cid,
  std::string (*get_name)() noexcept) const {

    return _get_storages<kind>().find(cid).and_then(func);
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, data_kind kind, typename Func>
auto basic_manager<Entity>::_apply_on_stg(const Func& func) const {
    return _apply_on_base_stg<kind>(
      [&func](auto& b_storage) {
          using S = storage<Entity, Component, kind>;

          S* ct_storage = dynamic_cast<S*>(b_storage.get());
          assert(ct_storage);

          return func(ct_storage);
      },
      Component::uid(),
      _cmp_name_getter<Component>());
}
//------------------------------------------------------------------------------
template <typename Entity>
template <data_kind kind>
auto basic_manager<Entity>::_get_stg_type_caps(
  identifier_t cid,
  std::string (*get_name)() noexcept) const noexcept -> storage_caps {
    return _apply_on_base_stg<kind>(
             [](auto& b_storage) -> always_valid<storage_caps> {
                 return b_storage->capabilities();
             },
             cid,
             get_name)
      .or_default();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_does_have_c(
  entity_param_t<Entity> ent,
  identifier_t cid,
  std::string (*get_name)() noexcept) noexcept -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&ent](auto& b_storage) -> tribool { return b_storage->has(ent); },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_does_have_r(
  entity_param_t<Entity> subject,
  entity_param_t<Entity> object,
  identifier_t cid,
  std::string (*get_name)() noexcept) noexcept -> bool {
    return _apply_on_base_stg<data_kind::relation>(
             [&subject, &object](auto& b_storage) -> tribool {
                 return b_storage->has(subject, object);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_is_hidn(
  entity_param_t<Entity> ent,
  identifier_t cid,
  std::string (*get_name)() noexcept) noexcept -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&ent](auto& b_storage) -> tribool {
                 return b_storage->is_hidden(ent);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_show(
  entity_param_t<Entity> ent,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&ent](auto& b_storage) -> tribool {
                 return b_storage->show(ent);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_hide(
  entity_param_t<Entity> ent,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&ent](auto& b_storage) -> tribool {
                 return b_storage->hide(ent);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component>
auto basic_manager<Entity>::_do_add_c(
  entity_param_t<Entity> ent,
  Component&& component) -> optional_reference<Component> {
    return _apply_on_stg<Component, data_kind::component>(
      [&ent, &component](auto& c_storage) -> optional_reference<Component> {
          return c_storage->store(ent, std::forward<Component>(component));
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Relation>
auto basic_manager<Entity>::_do_add_r(
  entity_param subj,
  entity_param obj,
  Relation&& relation) -> optional_reference<Relation> {
    return _apply_on_stg<Relation, data_kind::relation>(
      [&subj, &obj, &relation](
        auto& r_storage) -> optional_reference<Relation> {
          return r_storage->store(subj, obj, std::forward<Relation>(relation));
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_add_r(
  entity_param subject,
  entity_param object,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::relation>(
             [&subject, &object](auto& b_storage) -> tribool {
                 return b_storage->store(subject, object);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component>
auto basic_manager<Entity>::_do_cpy(
  entity_param_t<Entity> from,
  entity_param_t<Entity> to,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> optional_reference<Component> {
    return _apply_on_base_stg<data_kind::component>(
      [&from, &to](auto& b_storage) -> optional_reference<Component> {
          return static_cast<Component*>(b_storage->copy(from, to));
      },
      cid,
      get_name);
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_xchg(
  entity_param_t<Entity> e1,
  entity_param_t<Entity> e2,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&e1, &e2](auto& b_storage) -> tribool {
                 b_storage->exchange(e1, e2);
                 return true;
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_rem_c(
  entity_param_t<Entity> ent,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::component>(
             [&ent](auto& b_storage) -> tribool {
                 return b_storage->remove(ent);
             },
             cid,
             get_name)
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::_do_rem_r(
  entity_param_t<Entity> subj,
  entity_param_t<Entity> obj,
  identifier_t cid,
  std::string (*get_name)() noexcept) -> bool {
    return _apply_on_base_stg<data_kind::relation>(
             [&subj, &obj](auto& b_storage) -> tribool {
                 return b_storage->remove(subj, obj);
             },
             cid,
             get_name)
      .or_false();
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
template <typename Func, typename... M>
void basic_manager<Entity>::_call_for_single_c_p(
  mp_list<>,
  const Func& func,
  entity_param_t<Entity> ent,
  manipulator<M>&... m) {
    func(ent, m...);
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Func, typename C, typename... Cs, typename... M>
void basic_manager<Entity>::_call_for_single_c_p(
  mp_list<C, Cs...>,
  const Func& func,
  entity_param_t<Entity> ent,
  manipulator<M>&... m) {
    _apply_on_stg<std::remove_const_t<C>, data_kind::component>(
      [&, this](auto& c_storage) -> tribool {
          const auto hlpr{
            [&, this](entity_param_t<Entity> e, manipulator<C>& n) {
                _call_for_single_c_p(mp_list<Cs...>{}, func, e, m..., n);
            }};
          c_storage->for_single(
            callable_ref<void(entity_param, manipulator<C>&)>{
              construct_from, hlpr},
            ent);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, typename Func>
auto basic_manager<Entity>::_call_for_single_c(
  entity_param_t<Entity> ent,
  const Func& func) -> bool {
    return _apply_on_stg<std::remove_const_t<Component>, data_kind::component>(
             [&func, &ent](auto& c_storage) -> tribool {
                 c_storage->for_single(func, ent);
                 return true;
             })
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, typename Func>
void basic_manager<Entity>::_call_for_each_c(const Func& func) {
    _apply_on_stg<std::remove_const_t<Component>, data_kind::component>(
      [&func](auto& c_storage) -> tribool {
          c_storage->for_each(func);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Relation, typename Func>
void basic_manager<Entity>::_call_for_each_r(const Func& func) {
    _apply_on_stg<std::remove_const_t<Relation>, data_kind::relation>(
      [&func](auto& c_storage) -> tribool {
          c_storage->for_each(func);
          return true;
      })
      .or_false();
}
//------------------------------------------------------------------------------
template <typename Entity, typename C>
class _manager_for_each_c_m_base {
private:
    component_storage<Entity, std::remove_const_t<C>>& _storage;

protected:
    component_storage_iterator<Entity> _iter;
    Entity _curr;

    static constexpr auto _storage_buffer() noexcept {
        return storage_buffer_from_constness(std::is_const_v<C>);
    }

    _manager_for_each_c_m_base(
      component_storage<Entity, std::remove_const_t<C>>& strg)
      : _storage(strg)
      , _iter(_storage.new_iterator(_storage_buffer()))
      , _curr(_iter.done() ? Entity() : _iter.current()) {
        assert(std::is_const<C>::value or _storage.capabilities().can_modify());
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
        if(not _done()) {
            if(_current() == m) {
                _iter.next();
                if(not _done()) {
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
        if(this->_done() or (m < this->_current())) {
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
        return this->_done() and _rest.done();
    }

    void next_if_min(entity_param_t<Entity> m) {
        _rest.next_if_min(m);
        this->_next_if(m);
    }

    auto min_entity() -> Entity {
        if(_rest.done()) {
            assert(not this->_done());
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
        if(this->_done() or (m < this->_current())) {
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
        assert(not done());

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
    while(not hlp.done()) {
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
        if(not _iter.done()) {
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
        return this->_done() or _rest.done();
    }

    auto sync_to(entity_param_t<Entity> m) -> bool {
        return _rest.sync_to(m) and this->_find(m);
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
        return _rest.next() and this->_next();
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
        assert(not done());

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
        while(not hlp.done()) {
            hlp.apply();
            if(not hlp.next()) {
                break;
            }
            if(not hlp.sync()) {
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::knows(entity_param_t<Entity> ent) noexcept -> bool {
    for(auto& entry : _cmp_storages) {
        auto& storage{std::get<1>(entry)};
        if(storage and storage->has(ent)) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
template <typename Entity>
auto basic_manager<Entity>::spawn() noexcept -> Entity {
    do {
        _entity_sequence = entity_traits<Entity>::next(_entity_sequence);
    } while(knows(_entity_sequence));
    this->entity_spawned(_entity_sequence);
    return _entity_sequence;
}
//------------------------------------------------------------------------------
template <typename Entity>
void basic_manager<Entity>::forget(entity_param_t<Entity> ent) {
    if(ent) {
        for(auto& entry : _cmp_storages) {
            auto& storage{std::get<1>(entry)};
            if(storage) {
                if(storage->capabilities().can_remove()) {
                    storage->remove(ent);
                }
            }
        }
        this->entity_forgotten(ent);
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::ecs
