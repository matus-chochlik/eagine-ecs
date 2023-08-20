/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.ecs:std_map_storage;

import std;
import eagine.core.types;
import eagine.core.utility;
import eagine.core.container;
import eagine.core.reflection;
import :entity_traits;
import :manipulator;
import :storage;

namespace eagine::ecs {
//------------------------------------------------------------------------------
export template <typename Entity, typename Component>
class std_map_cmp_storage;

export template <typename Entity, typename Component>
class std_map_cmp_storage_iterator
  : public component_storage_iterator_intf<Entity> {
    using _map_t = std::map<Entity, Component>;

public:
    std_map_cmp_storage_iterator(_map_t& m) noexcept
      : _map{&m}
      , _i{m.begin()} {
        assert(_map);
    }

    void reset() override {
        assert(_map);
        _i = _map->begin();
    }

    auto done() -> bool override {
        assert(_map);
        return _i == _map->end();
    }

    void next() override {
        assert(not done());
        ++_i;
    }

    auto find(entity_param_t<Entity> e) -> bool override {
        if(done()) {
            return false;
        }
        if(e == _i->first) {
            return true;
        }
        if(e < _i->first) {
            return false;
        }

        while(++_i != _map->end()) {
            if(_i->first == e) {
                return true;
            }
        }
        return false;
    }

    auto current() -> Entity override {
        return _i->first;
    }

private:
    using _iter_t = typename _map_t::iterator;
    _map_t* _map{nullptr};
    _iter_t _i;

    friend class std_map_cmp_storage<Entity, Component>;
};
//------------------------------------------------------------------------------
export template <typename Entity, typename Component>
class std_map_cmp_storage : public component_storage<Entity, Component> {
public:
    using entity_param = entity_param_t<Entity>;
    using iterator_t = component_storage_iterator<Entity>;

    auto capabilities() -> storage_caps override {
        return storage_caps{
          storage_cap_bit::hide | storage_cap_bit::remove |
          storage_cap_bit::store | storage_cap_bit::modify};
    }

    auto new_iterator() -> iterator_t override {
        return iterator_t(_iterators.make(_components));
    }

    void delete_iterator(iterator_t&& i) override {
        _iterators.eat(i.release());
    }

    auto has(entity_param e) -> bool override {
        return _components.contains(e);
    }

    auto is_hidden(entity_param e) -> bool override {
        return _hidden.contains(e);
    }

    auto is_hidden(iterator_t& i) -> bool override {
        assert(not i.done());
        return is_hidden(_iter_entity(i));
    }

    auto hide(entity_param e) -> bool override {
        if(has(e)) {
            _hidden.insert(e);
            return true;
        }
        return false;
    }

    void hide(iterator_t& i) override {
        assert(not i.done());
        _hidden.insert(_iter_entity(i));
    }

    auto show(entity_param e) -> bool override {
        return _hidden.erase(e) > 0;
    }

    auto show(iterator_t& i) -> bool override {
        return _hidden.erase(_iter_entity(i)) > 0;
    }

    auto copy(entity_param ef, entity_param et) -> void* override {
        if(is_hidden(ef)) {
            return nullptr;
        }
        if(const auto found{find(_components, ef)}) {
            return static_cast<void*>(store(et, Component(*found)));
        }
        return nullptr;
    }

    auto swap(const entity_param ea, const entity_param eb) -> bool override {
        const auto fa{find(_components, ea)};
        const auto fb{find(_components, eb)};
        const bool ha{is_hidden(ea)};
        const bool hb{is_hidden(eb)};

        if(fa and fb) {
            using std::swap;
            swap(*fa, *fb);
            if(ha and not hb) {
                show(ea);
            }
            if(hb and not ha) {
                show(eb);
            }
        } else if(fa) {
            store(eb, std::move(*fa));
            remove(ea);
            if(ha) {
                hide(eb);
            }
        } else if(fb) {
            store(ea, std::move(*fb));
            remove(eb);
            if(hb) {
                hide(ea);
            }
        }
        return true;
    }

    auto remove(entity_param e) -> bool override {
        _hidden.erase(e);
        return _components.erase(e) > 0;
    }

    void remove(iterator_t& i) override {
        assert(not i.done());
        _hidden.erase(_iter_entity(i));
        _iter_cast(i)._i = _components.erase(_iter_cast(i)._i);
    }

    auto store(entity_param e, Component&& c) -> Component* override {
        _hidden.erase(e);
        const auto pos = _components.emplace(e, std::move(c)).first;
        return &pos->second;
    }

    auto store(iterator_t& i, entity_param e, Component&& c)
      -> Component* override {
        _hidden.erase(e);
        auto& pos = _iter_cast(i)._i;
        pos = _components.emplace_hint(pos, e, std::move(c));
        return &pos->second;
    }

    void for_single(
      const callable_ref<void(entity_param, manipulator<const Component>&)> func,
      entity_param e) override {
        if(auto found{eagine::find(_components, e)}) {
            if(not is_hidden(e)) {
                concrete_manipulator<const Component> m(
                  *found, true /*can_remove*/);
                func(e, m);
                if(m.remove_requested()) {
                    _remove(found.position());
                }
            }
        }
    }

    void for_single(
      const callable_ref<void(entity_param, manipulator<const Component>&)> func,
      iterator_t& i) override {
        assert(not i.done());
        auto& p = _iter_cast(i)._i;
        assert(p != _components.end());
        if(not is_hidden(p->first)) {
            concrete_manipulator<const Component> m(
              p->second, true /*can_remove*/);
            func(p->first, m);
            if(m.remove_requested()) {
                p = _remove(p);
            }
        }
    }

    void for_single(
      const callable_ref<void(entity_param, manipulator<Component>&)> func,
      entity_param e) override {
        if(auto found{eagine::find(_components, e)}) {
            if(not is_hidden(e)) {
                // TODO: modify notification
                concrete_manipulator<Component> m(*found, true /*can_remove*/);
                func(e, m);
                if(m.remove_requested()) {
                    _remove(found.position());
                }
            }
        }
    }

    void for_single(
      const callable_ref<void(entity_param, manipulator<Component>&)> func,
      iterator_t& i) override {
        assert(not i.done());
        auto& p = _iter_cast(i)._i;
        assert(p != _components.end());
        if(not is_hidden(p->first)) {
            // TODO: modify notification
            concrete_manipulator<Component> m(
              p->second, true /*can_remove*/
            );
            func(p->first, m);
            if(m.remove_requested()) {
                p = _remove(p);
            }
        }
    }

    void for_each(
      const callable_ref<void(entity_param, manipulator<const Component>&)>
        func) override {
        concrete_manipulator<const Component> m(true /*can_remove*/);
        auto p = _components.begin();
        while(p != _components.end()) {
            if(not is_hidden(p->first)) {
                m.reset(p->second);
                func(p->first, m);
                if(m.remove_requested()) {
                    p = _remove(p);
                } else {
                    ++p;
                }
            } else {
                ++p;
            }
        }
    }

    void for_each(
      const callable_ref<void(entity_param, manipulator<Component>&)> func)
      override {
        concrete_manipulator<Component> m(true /*can_remove*/);
        auto p = _components.begin();
        while(p != _components.end()) {
            if(not is_hidden(p->first)) {
                // TODO: modify notification
                m.reset(p->second);
                func(p->first, m);
                if(m.remove_requested()) {
                    p = _remove(p);
                } else {
                    ++p;
                }
            } else {
                ++p;
            }
        }
    }

    void for_each(
      const callable_ref<void(manipulator<Component>&)> func) override {
        concrete_manipulator<Component> m(true /*can_remove*/);
        auto p = _components.begin();
        while(p != _components.end()) {
            if(not is_hidden(p->first)) {
                // TODO: modify notification
                m.reset(p->second);
                func(m);
                if(m.remove_requested()) {
                    p = _remove(p);
                } else {
                    ++p;
                }
            } else {
                ++p;
            }
        }
    }

private:
    using _map_iter_t = std_map_cmp_storage_iterator<Entity, Component>;

    std::map<Entity, Component> _components{};
    std::set<Entity> _hidden{};
    object_pool<_map_iter_t, 2> _iterators{};

    auto _iter_cast(component_storage_iterator<Entity>& i) noexcept -> auto& {
        assert(dynamic_cast<_map_iter_t*>(i.ptr()));
        return *static_cast<_map_iter_t*>(i.ptr());
    }

    auto _iter_entity(component_storage_iterator<Entity>& i) noexcept {
        return _iter_cast(i)._i->first;
    }

    auto _remove(typename std::map<Entity, Component>::iterator p) {
        assert(p != _components.end());
        _hidden.erase(p->first);
        return _components.erase(p);
    }
};
//------------------------------------------------------------------------------
export template <typename Entity, typename Relation>
class std_map_rel_storage;

export template <typename Entity, typename Relation>
class std_map_rel_storage_iterator
  : public relation_storage_iterator_intf<Entity> {
    using _pair_t = std::pair<Entity, Entity>;
    using _map_t = std::map<_pair_t, Relation>;

public:
    std_map_rel_storage_iterator(_map_t& m) noexcept
      : _map{&m}
      , _i(m.begin()) {
        assert(_map);
    }

    void reset() override {
        assert(_map);
        _i = _map->begin();
    }

    auto done() -> bool override {
        assert(_map);
        return _i == _map->end();
    }

    void next() override {
        assert(not done());
        ++_i;
    }

    auto subject() -> Entity override {
        return _i->first.first;
    }

    auto object() -> Entity override {
        return _i->first.second;
    }

private:
    using _iter_t = typename _map_t::iterator;
    _map_t* _map{nullptr};
    _iter_t _i;

    friend class std_map_rel_storage<Entity, Relation>;
};
//------------------------------------------------------------------------------
export template <typename Entity, typename Relation>
class std_map_rel_storage : public relation_storage<Entity, Relation> {
public:
    using entity_param = entity_param_t<Entity>;
    using iterator_t = relation_storage_iterator<Entity>;

    auto capabilities() -> storage_caps override {
        return storage_caps{
          storage_cap_bit::remove | storage_cap_bit::store |
          storage_cap_bit::modify};
    }

    auto new_iterator() -> iterator_t override {
        return iterator_t(_iterators.make(_relations));
    }

    void delete_iterator(iterator_t&& i) override {
        _iterators.eat(i.release());
    }

    auto has(entity_param s, entity_param o) -> bool override {
        return _relations.contains(_pair_t(s, o));
    }

    auto store(entity_param s, entity_param o) -> bool override {
        _relations.emplace(_pair_t(s, o), Relation());
        return true;
    }

    auto store(entity_param s, entity_param o, Relation&& r)
      -> Relation* override {
        const auto pos = _relations.emplace(_pair_t(s, o), std::move(r)).first;
        return &pos->second;
    }

    auto remove(entity_param s, entity_param o) -> bool override {
        return _relations.erase(_pair_t(s, o)) > 0;
    }

    void remove(iterator_t& i) override {
        assert(not i.done());
        _iter_cast(i)._i = _relations.erase(_iter_cast(i)._i);
    }

    void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      entity_param subject,
      entity_param object) override {
        if(const auto found{find(_relations, _pair_t(subject, object))}) {
            concrete_manipulator<const Relation> m(*found, true /*can_erase*/);
            func(subject, object, m);
            if(m.remove_requested()) {
                _remove(found.position());
            }
        }
    }

    void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      iterator_t& i) override {
        assert(not i.done());
        auto& po = _iter_cast(i)._i;
        assert(po != _relations.end());

        concrete_manipulator<const Relation> m(
          po->second, true /*can_erase*/
        );
        func(po->first.first, po->first.second, m);
        if(m.remove_requested()) {
            po = _remove(po);
        }
    }

    void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)> func,
      entity_param subject,
      entity_param object) override {
        if(const auto found{find(_relations, _pair_t(subject, object))}) {
            // TODO: modify notification
            concrete_manipulator<Relation> m(*found, true /*can_erase*/);
            func(subject, object, m);
            if(m.remove_requested()) {
                _remove(found.position());
            }
        }
    }

    void for_single(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)> func,
      iterator_t& i) override {
        assert(not i.done());
        auto& po = _iter_cast(i)._i;
        assert(po != _relations.end());

        // TODO: modify notification
        concrete_manipulator<Relation> m(
          po->second, true /*can_erase*/
        );
        func(po->first.first, po->first.second, m);
        if(m.remove_requested()) {
            po = _remove(po);
        }
    }

    void for_each(
      const callable_ref<void(entity_param, entity_param)> func,
      entity_param subject) override {
        entity_param object = entity_traits<Entity>::first();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) and (po->first.first == subject)) {
            func(subject, po->first.second);
            ++po;
        }
    }

    void for_each(
      const callable_ref<void(entity_param, entity_param)> func) override {
        for(auto& p : _relations) {
            func(p.first.first, p.first.second);
        }
    }

    void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      entity_param subject) override {
        concrete_manipulator<const Relation> m(true /*can_remove*/);
        entity_param object = entity_traits<Entity>::first();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) and (po->first.first == subject)) {
            m.reset(po->second);
            func(subject, po->first.second, m);
            if(m.remove_requested()) {
                po = _remove(po);
            } else {
                ++po;
            }
        }
    }

    void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<Relation>&)> func,
      entity_param subject) override {
        concrete_manipulator<Relation> m(true /*can_remove*/);
        entity_param object = entity_traits<Entity>::first();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) and (po->first.first == subject)) {
            // TODO: modify notification
            m.reset(po->second);
            func(subject, po->first.second, m);
            if(m.remove_requested()) {
                po = _remove(po);
            } else {
                ++po;
            }
        }
    }

    void for_each(
      const callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func)
      override {
        concrete_manipulator<const Relation> m(true /*can_remove*/);
        auto po = _relations.begin();
        while(po != _relations.end()) {
            m.reset(po->second);
            func(po->first.first, po->first.second, m);
            if(m.remove_requested()) {
                po = _remove(po);
            } else {
                ++po;
            }
        }
    }

    void for_each(const callable_ref<
                  void(entity_param, entity_param, manipulator<Relation>&)>
                    func) override {
        concrete_manipulator<Relation> m(true /*can_remove*/);
        auto po = _relations.begin();
        while(po != _relations.end()) {
            // TODO: modify notification
            m.reset(po->second);
            func(po->first.first, po->first.second, m);
            if(m.remove_requested()) {
                po = _remove(po);
            } else {
                ++po;
            }
        }
    }

private:
    using _pair_t = std::pair<Entity, Entity>;
    using _map_iter_t = std_map_rel_storage_iterator<Entity, Relation>;

    std::map<_pair_t, Relation> _relations;
    object_pool<_map_iter_t, 2> _iterators{};

    auto _iter_cast(relation_storage_iterator<Entity>& i) noexcept -> auto& {
        assert(dynamic_cast<_map_iter_t*>(i.ptr()) != nullptr);
        return *static_cast<_map_iter_t*>(i.ptr());
    }

    auto _remove(typename std::map<_pair_t, Relation>::iterator p) {
        assert(p != _relations.end());
        return _relations.erase(p);
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::ecs

