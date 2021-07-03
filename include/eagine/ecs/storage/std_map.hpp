/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_ECS_STORAGE_STD_MAP_HPP
#define EAGINE_ECS_STORAGE_STD_MAP_HPP

#include "../cmp_storage.hpp"
#include "../rel_storage.hpp"
#include <eagine/assert.hpp>
#include <map>
#include <set>

namespace eagine {
namespace ecs {

template <typename Entity, typename Component>
class std_map_cmp_storage;

template <typename Entity, typename Component>
class std_map_cmp_storage_iterator
  : public component_storage_iterator_intf<Entity> {
private:
    using _map_t = typename std::map<Entity, Component>;
    using _iter_t = typename _map_t::iterator;
    _map_t* _map{nullptr};
    _iter_t _i;

    friend class std_map_cmp_storage<Entity, Component>;

public:
    std_map_cmp_storage_iterator(_map_t& m) noexcept
      : _map{&m}
      , _i{m.begin()} {
        EAGINE_ASSERT(_map);
    }

    void reset() override {
        EAGINE_ASSERT(_map);
        _i = _map->begin();
    }

    auto done() -> bool override {
        EAGINE_ASSERT(_map);
        return _i == _map->end();
    }

    void next() override {
        EAGINE_ASSERT(!done());
        ++_i;
    }

    auto find(Entity e) -> bool override {
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
};

template <typename Entity, typename Component>
class std_map_cmp_storage : public component_storage<Entity, Component> {
private:
    std::map<Entity, Component> _components{};
    std::set<Entity> _hidden{};

    using _map_iter_t = std_map_cmp_storage_iterator<Entity, Component>;

    auto _iter_cast(component_storage_iterator<Entity>& i) noexcept -> auto& {
        EAGINE_ASSERT(dynamic_cast<_map_iter_t*>(i.ptr()));
        return *static_cast<_map_iter_t*>(i.ptr());
    }

    auto _iter_entity(component_storage_iterator<Entity>& i) noexcept {
        return _iter_cast(i)._i->first;
    }

    auto _remove(typename std::map<Entity, Component>::iterator p) {
        EAGINE_ASSERT(p != _components.end());
        _hidden.erase(p->first);
        return _components.erase(p);
    }

public:
    using entity_param = entity_param_t<Entity>;
    using iterator_t = component_storage_iterator<Entity>;

    auto capabilities() -> storage_caps override {
        return storage_caps{
          storage_cap_bit::hide | storage_cap_bit::remove |
          storage_cap_bit::store | storage_cap_bit::modify};
    }

    auto new_iterator() -> iterator_t override {
        return iterator_t(new _map_iter_t(_components));
    }

    void delete_iterator(iterator_t&& i) override {
        delete i.release();
    }

    auto has(entity_param e) -> bool override {
        return _components.find(e) != _components.end();
    }

    auto is_hidden(entity_param e) -> bool override {
        return _hidden.find(e) != _hidden.end();
    }

    auto is_hidden(iterator_t& i) -> bool override {
        EAGINE_ASSERT(!i.done());
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
        EAGINE_ASSERT(!i.done());
        _hidden.insert(_iter_entity(i));
    }

    auto show(entity_param e) -> bool override {
        return _hidden.erase(e) > 0;
    }

    auto show(iterator_t& i) -> bool override {
        return _hidden.erase(_iter_entity(i)) > 0;
    }

    auto copy(entity_param ef, entity_param et) -> bool override {
        if(is_hidden(ef)) {
            return false;
        }
        auto pf = _components.find(ef);
        if(pf == _components.end()) {
            return false;
        }
        return store(et, Component(pf->second));
    }

    auto swap(entity_param ea, entity_param eb) -> bool override {
        auto pa = _components.find(ea);
        auto pb = _components.find(eb);
        bool ha = is_hidden(ea);
        bool hb = is_hidden(eb);

        if(pa != _components.end() && pb != _components.end()) {
            using std::swap;
            swap(pa->second, pb->second);
            if(ha && !hb) {
                show(ea);
            }
            if(hb && !ha) {
                show(eb);
            }
        } else if(pa != _components.end()) {
            store(eb, std::move(pa->second));
            remove(ea);
            if(ha) {
                hide(eb);
            }
        } else if(pb != _components.end()) {
            store(ea, std::move(pb->second));
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
        EAGINE_ASSERT(!i.done());
        _hidden.erase(_iter_entity(i));
        _iter_cast(i)._i = _components.erase(_iter_cast(i)._i);
    }

    auto store(entity_param e, Component&& c) -> bool override {
        _hidden.erase(e);
        _components.emplace(e, std::move(c));
        return true;
    }

    auto store(iterator_t& i, entity_param e, Component&& c) -> bool override {
        _hidden.erase(e);
        auto& p = _iter_cast(i)._i;
        p = _components.emplace_hint(p, e, std::move(c));
        return true;
    }

    void for_single(
      callable_ref<void(entity_param, manipulator<const Component>&)> func,
      entity_param e) override {
        auto p = _components.find(e);
        if(p != _components.end()) {
            if(!is_hidden(e)) {
                concrete_manipulator<const Component> m(
                  p->second, true /*can_remove*/
                );
                func(p->first, m);
                if(m.remove_requested()) {
                    _remove(p);
                }
            }
        }
    }

    void for_single(
      callable_ref<void(entity_param, manipulator<const Component>&)> func,
      iterator_t& i) override {
        EAGINE_ASSERT(!i.done());
        auto& p = _iter_cast(i)._i;
        EAGINE_ASSERT(p != _components.end());
        if(!is_hidden(p->first)) {
            concrete_manipulator<const Component> m(
              p->second, true /*can_remove*/
            );
            func(p->first, m);
            if(m.remove_requested()) {
                p = _remove(p);
            }
        }
    }

    void for_single(
      callable_ref<void(entity_param, manipulator<Component>&)> func,
      entity_param e) override {
        auto p = _components.find(e);
        if(p != _components.end()) {
            if(!is_hidden(e)) {
                // TODO: modify notification
                concrete_manipulator<Component> m(
                  p->second, true /*can_remove*/
                );
                func(p->first, m);
                if(m.remove_requested()) {
                    _remove(p);
                }
            }
        }
    }

    void for_single(
      callable_ref<void(entity_param, manipulator<Component>&)> func,
      iterator_t& i) override {
        EAGINE_ASSERT(!i.done());
        auto& p = _iter_cast(i)._i;
        EAGINE_ASSERT(p != _components.end());
        if(!is_hidden(p->first)) {
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
      callable_ref<void(entity_param, manipulator<const Component>&)> func)
      override {
        concrete_manipulator<const Component> m(true /*can_remove*/);
        auto p = _components.begin();
        while(p != _components.end()) {
            if(!is_hidden(p->first)) {
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
      callable_ref<void(entity_param, manipulator<Component>&)> func) override {
        concrete_manipulator<Component> m(true /*can_remove*/);
        auto p = _components.begin();
        while(p != _components.end()) {
            if(!is_hidden(p->first)) {
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
};

template <typename Entity, typename Relation>
class std_map_rel_storage;

template <typename Entity, typename Relation>
class std_map_rel_storage_iterator
  : public relation_storage_iterator_intf<Entity> {
private:
    using _pair_t = std::pair<Entity, Entity>;
    using _map_t = typename std::map<_pair_t, Relation>;
    using _iter_t = typename _map_t::iterator;
    _map_t* _map{nullptr};
    _iter_t _i;

    friend class std_map_rel_storage<Entity, Relation>;

public:
    std_map_rel_storage_iterator(_map_t& m) noexcept
      : _map{&m}
      , _i(m.begin()) {
        EAGINE_ASSERT(_map);
    }

    void reset() override {
        EAGINE_ASSERT(_map);
        _i = _map->begin();
    }

    auto done() -> bool override {
        EAGINE_ASSERT(_map);
        return _i == _map->end();
    }

    void next() override {
        EAGINE_ASSERT(!done());
        ++_i;
    }

    auto subject() -> Entity override {
        return _i->first.first;
    }

    auto object() -> Entity override {
        return _i->first.second;
    }
};

template <typename Entity, typename Relation>
class std_map_rel_storage : public relation_storage<Entity, Relation> {
private:
    using _pair_t = std::pair<Entity, Entity>;
    std::map<_pair_t, Relation> _relations;

    using _map_iter_t = std_map_rel_storage_iterator<Entity, Relation>;

    auto _iter_cast(relation_storage_iterator<Entity>& i) noexcept -> auto& {
        EAGINE_ASSERT(dynamic_cast<_map_iter_t*>(i.ptr()) != nullptr);
        return *static_cast<_map_iter_t*>(i.ptr());
    }

    auto _remove(typename std::map<_pair_t, Relation>::iterator p) {
        EAGINE_ASSERT(p != _relations.end());
        return _relations.erase(p);
    }

public:
    using entity_param = entity_param_t<Entity>;
    using iterator_t = relation_storage_iterator<Entity>;

    auto capabilities() -> storage_caps override {
        return storage_caps{
          storage_cap_bit::remove | storage_cap_bit::store |
          storage_cap_bit::modify};
    }

    auto new_iterator() -> iterator_t override {
        return iterator_t(new _map_iter_t(_relations));
    }

    void delete_iterator(iterator_t&& i) override {
        delete i.release();
    }

    auto has(entity_param s, entity_param o) -> bool override {
        return _relations.find(_pair_t(s, o)) != _relations.end();
    }

    auto store(entity_param s, entity_param o) -> bool override {
        _relations.emplace(_pair_t(s, o), Relation());
        return true;
    }

    auto store(entity_param s, entity_param o, Relation&& r) -> bool override {
        _relations.emplace(_pair_t(s, o), std::move(r));
        return true;
    }

    auto remove(entity_param s, entity_param o) -> bool override {
        return _relations.erase(_pair_t(s, o)) > 0;
    }

    void remove(iterator_t& i) override {
        EAGINE_ASSERT(!i.done());
        _iter_cast(i)._i = _relations.erase(_iter_cast(i)._i);
    }

    void for_single(
      callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      entity_param subject,
      entity_param object) override {
        auto po = _relations.find(_pair_t(subject, object));
        if(po != _relations.end()) {
            concrete_manipulator<const Relation> m(
              po->second, true /*can_erase*/
            );
            func(po->first.first, po->first.second, m);
            if(m.remove_requested()) {
                _remove(po);
            }
        }
    }

    void for_single(
      callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      iterator_t& i) override {
        EAGINE_ASSERT(!i.done());
        auto& po = _iter_cast(i)._i;
        EAGINE_ASSERT(po != _relations.end());

        concrete_manipulator<const Relation> m(
          po->second, true /*can_erase*/
        );
        func(po->first.first, po->first.second, m);
        if(m.remove_requested()) {
            po = _remove(po);
        }
    }

    void for_single(
      callable_ref<void(entity_param, entity_param, manipulator<Relation>&)>
        func,
      entity_param subject,
      entity_param object) override {
        auto po = _relations.find(_pair_t(subject, object));
        if(po != _relations.end()) {
            // TODO: modify notification
            concrete_manipulator<Relation> m(
              po->second, true /*can_erase*/
            );
            func(po->first.first, po->first.second, m);
            if(m.remove_requested()) {
                _remove(po);
            }
        }
    }

    void for_single(
      callable_ref<void(entity_param, entity_param, manipulator<Relation>&)>
        func,
      iterator_t& i) override {
        EAGINE_ASSERT(!i.done());
        auto& po = _iter_cast(i)._i;
        EAGINE_ASSERT(po != _relations.end());

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
      callable_ref<void(entity_param, entity_param)> func,
      entity_param subject) override {
        entity_param object = entity_traits<Entity>::minimum();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) && (po->first.first == subject)) {
            func(subject, po->first.second);
            ++po;
        }
    }

    void for_each(callable_ref<void(entity_param, entity_param)> func) override {
        for(auto& p : _relations) {
            func(p.first.first, p.first.second);
        }
    }

    void for_each(
      callable_ref<
        void(entity_param, entity_param, manipulator<const Relation>&)> func,
      entity_param subject) override {
        concrete_manipulator<const Relation> m(true /*can_remove*/);
        entity_param object = entity_traits<Entity>::minimum();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) && (po->first.first == subject)) {
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
      callable_ref<void(entity_param, entity_param, manipulator<Relation>&)>
        func,
      entity_param subject) override {
        concrete_manipulator<Relation> m(true /*can_remove*/);
        entity_param object = entity_traits<Entity>::minimum();
        auto po = _relations.lower_bound(_pair_t(subject, object));
        while((po != _relations.end()) && (po->first.first == subject)) {
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
      callable_ref<void(entity_param, entity_param, manipulator<const Relation>&)>
        func) override {
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

    void for_each(
      callable_ref<void(entity_param, entity_param, manipulator<Relation>&)>
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
};

} // namespace ecs
} // namespace eagine

#endif // EAGINE_ECS_STORAGE_STD_MAP_HPP
