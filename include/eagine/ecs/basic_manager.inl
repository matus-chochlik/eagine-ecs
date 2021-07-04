/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include <eagine/assert.hpp>

#if !EAGINE_ECS_LIBRARY || defined(EAGINE_IMPLEMENTING_LIBRARY)
#include <eagine/str_format.hpp>
#include <stdexcept>
#endif

namespace eagine::ecs {
//------------------------------------------------------------------------------
namespace detail {
#if !EAGINE_ECS_LIBRARY || defined(EAGINE_IMPLEMENTING_LIBRARY)
//------------------------------------------------------------------------------
[[noreturn]] EAGINE_LIB_FUNC void mgr_handle_cmp_is_reg(std::string&& c_name) {
    throw std::runtime_error(
      format("Component type '${1}' is already registered") %
      std::move(c_name));
}
//------------------------------------------------------------------------------
[[noreturn]] EAGINE_LIB_FUNC void mgr_handle_cmp_not_reg(std::string&& c_name) {
    throw std::runtime_error(
      format("Component type '${1}' is not registered") % std::move(c_name));
}
//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------
[[noreturn]] void mgr_handle_cmp_is_reg(std::string&&);
//------------------------------------------------------------------------------
[[noreturn]] void mgr_handle_cmp_not_reg(std::string&&);
//------------------------------------------------------------------------------
#endif
} // namespace detail
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
            EAGINE_ASSERT(pd_storage);
        }
    }
    if(!pd_storage) {
        std::string (*get_name)() = _cmp_name_getter<Data>();
        detail::mgr_handle_cmp_not_reg(get_name());
        EAGINE_ABORT("Logic error!");
    }
    return *pd_storage;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation>
inline void basic_manager<Entity>::_do_reg_stg_type(
  std::unique_ptr<base_storage<Entity, IsRelation>>&& storage,
  component_uid_t cid,
  std::string (*get_name)()) {
    EAGINE_ASSERT(bool(storage));

    if(!_get_storages<IsRelation>().emplace(cid, std::move(storage))) {
        detail::mgr_handle_cmp_is_reg(get_name());
    }
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation>
inline void basic_manager<Entity>::_do_unr_stg_type(
  component_uid_t cid,
  std::string (*get_name)()) {

    if(_get_storages<IsRelation>().erase(cid) != 1) {
        detail::mgr_handle_cmp_not_reg(get_name());
    }
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation>
inline auto
basic_manager<Entity>::_does_know_stg_type(component_uid_t cid) const -> bool {

    return _get_storages<IsRelation>().find(cid).is_valid();
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation, typename Result, typename Func>
inline auto basic_manager<Entity>::_apply_on_base_stg(
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
    detail::mgr_handle_cmp_not_reg(get_name());
    return fallback;
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Component, bool IsRelation, typename Result, typename Func>
inline auto
basic_manager<Entity>::_apply_on_stg(Result fallback, const Func& func) const
  -> Result {
    return _apply_on_base_stg<IsRelation>(
      fallback,
      [&func](auto& b_storage) -> Result {
          using S = storage<Entity, Component, IsRelation>;

          S* ct_storage = dynamic_cast<S*>(b_storage.get());
          EAGINE_ASSERT(ct_storage);

          return func(ct_storage);
      },
      Component::uid(),
      _cmp_name_getter<Component>());
}
//------------------------------------------------------------------------------
template <typename Entity>
template <bool IsRelation>
inline auto basic_manager<Entity>::_get_stg_type_caps(
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
inline auto basic_manager<Entity>::_does_have_c(
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
inline auto basic_manager<Entity>::_does_have_r(
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
inline auto basic_manager<Entity>::_is_hidn(
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
inline auto basic_manager<Entity>::_do_show(
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
inline auto basic_manager<Entity>::_do_hide(
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
inline auto basic_manager<Entity>::_do_add_c(
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
inline auto basic_manager<Entity>::_do_add_r(
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
inline auto basic_manager<Entity>::_do_add_r(
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
inline auto basic_manager<Entity>::_do_cpy(
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
inline auto basic_manager<Entity>::_do_swp(
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
inline auto basic_manager<Entity>::_do_rem_c(
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
inline auto basic_manager<Entity>::_do_rem_r(
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
inline auto
basic_manager<Entity>::_do_get_c(T C::*mvp, entity_param_t<Entity> ent, T res)
  -> T {
    EAGINE_ASSERT(mvp);

    using MC = manipulator<const C>;

    auto getter = [mvp, &res](entity_param_t<Entity>, MC& cmp) {
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
inline auto basic_manager<Entity>::_call_for_single_c(
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
inline void basic_manager<Entity>::_call_for_each_c(const Func& func) {
    _apply_on_stg<std::remove_const_t<Component>, false>(
      false, [&func](auto& c_storage) -> bool {
          c_storage->for_each(func);
          return true;
      });
}
//------------------------------------------------------------------------------
template <typename Entity>
template <typename Relation, typename Func>
inline void basic_manager<Entity>::_call_for_each_r(const Func& func) {
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
      component_storage<Entity, std::remove_const_t<C>>& storage)
      : _storage(storage)
      , _iter(_storage.new_iterator())
      , _curr(_iter.done() ? Entity() : _iter.current()) {
        EAGINE_ASSERT(
          std::is_const<C>::value || _storage.capabilities().can_modify());
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
      component_storage<Entity, std::remove_const_t<C>>& storage)
      : _manager_for_each_c_m_base<Entity, C>(storage) {}

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
            EAGINE_ASSERT(m == this->_current());
            auto hlpr = [&clm..., this](
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
            EAGINE_ASSERT(!this->_done());
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

    void apply(entity_param_t<Entity> m, manipulator<CL>&... clm) {
        if(this->_done() || (m < this->_current())) {
            std::remove_const_t<C> cadd;
            concrete_manipulator<C> cman(nullptr, cadd, false);
            _rest.apply(m, clm..., cman);
            if(cman.add_requested()) {
                this->_store(m, std::move(cadd));
            }
        } else {
            EAGINE_ASSERT(m == this->_current());
            EAGINE_MAYBE_UNUSED(m);
            auto hlpr = [&clm..., this](
                          entity_param_t<Entity> e, manipulator<C>& cm) {
                _rest.apply(e, clm..., cm);
            };
            this->_apply({construct_from, hlpr});
        }
    }

    void apply() {
        static_assert(sizeof...(CL) == 0);
        EAGINE_ASSERT(!done());

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
inline void basic_manager<Entity>::_call_for_each_c_m_p(const Func& func) {
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

    void apply(entity_param_t<Entity> m, manipulator<CL>&... clm) {
        EAGINE_ASSERT(m == this->_current());
        EAGINE_MAYBE_UNUSED(m);
        auto hlpr = [&clm..., this](
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

    void apply(entity_param_t<Entity> m, manipulator<CL>&... clm) {
        EAGINE_ASSERT(m == this->_current());
        EAGINE_MAYBE_UNUSED(m);
        auto hlpr = [&clm..., this](
                      entity_param_t<Entity> e, manipulator<C>& cm) {
            _rest.apply(e, clm..., cm);
        };
        this->_apply({construct_from, hlpr});
    }

    void apply() {
        static_assert(sizeof...(CL) == 0);
        EAGINE_ASSERT(!done());

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
inline void basic_manager<Entity>::_call_for_each_c_m_r(const Func& func) {
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
