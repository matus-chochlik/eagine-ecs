/// @example eagine/ecs/000_particles.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.ecs;
import std;

namespace eagine {
//------------------------------------------------------------------------------
// components
//------------------------------------------------------------------------------
struct mass : ecs::component<"mass"> {
    tagged_quantity<float, units::kilogram> val{};
};
//------------------------------------------------------------------------------
struct position : ecs::component<"position"> {
    tagged_quantity<math::tvec<float, 3>, units::meter> vec{};
};
//------------------------------------------------------------------------------
struct velocity : ecs::component<"velocity"> {
    tagged_quantity<
      math::tvec<float, 3>,
      decltype(units::meter() / units::second())>
      vec{};
};
//------------------------------------------------------------------------------
struct all_forces : ecs::component<"allForces"> {
    tagged_quantity<math::tvec<float, 3>, units::newton> vec{};
};
//------------------------------------------------------------------------------
struct attraction : ecs::component<"attraction"> {
    tagged_quantity<math::tvec<float, 3>, units::newton> vec{};
};
//------------------------------------------------------------------------------
struct repulsion : ecs::component<"repulsion"> {
    tagged_quantity<math::tvec<float, 3>, units::newton> vec{};
};
//------------------------------------------------------------------------------
// relations
//------------------------------------------------------------------------------
struct spring : ecs::relation<"spring"> {
    float length{1};
    float strength{1};
};
//------------------------------------------------------------------------------
// systems
//------------------------------------------------------------------------------
class physics {
public:
private:
    void _calc_rep_attr(
      const identifier_t,
      ecs::manipulator<attraction>& attr,
      ecs::manipulator<repulsion>& repl,
      ecs::manipulator<const position>& l,
      const identifier_t,
      ecs::manipulator<const position>& r,
      ecs::manipulator<const spring>& s) {
        const auto distance{distance(l->p, r->p)};
        attr->vec = {};
        repl->vec = {};
    }

    /*
    void _do_update_velocity(
      const identifier_t,
      ecs::manipulator<velocity>& vel,
      ecs::manipulator<const acceleration>& acc) {
        vel->v = vel->v + acc->a * _delta_t;
    }
    const member_callable_ref_t<&physics::_do_update_velocity> _update_velocity{
      this};

    void _do_update_position(
      const identifier_t,
      ecs::manipulator<position>& pos,
      ecs::manipulator<const velocity>& vel) {
        pos->p = pos->p + vel->v * _delta_t;
    }
    const member_callable_ref_t<&physics::_do_update_position> _update_position{
      this};
      */

    tagged_quantity<float, units::second> _delta_t{0.01F};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    using namespace eagine;
    ctx.cio().print(identifier{"ECS"}, "starting");

    auto& mgr = enable_ecs(ctx);

    mgr.register_component_storages<
      ecs::std_map_cmp_storage,
      mass,
      position,
      velocity,
      attraction,
      repulsion,
      all_forces>();
    mgr.register_relation_storages<ecs::std_map_rel_storage, spring>();

    /*
     *    (C)-----(D)    (Y)
     *    /|      /|      ^
     *   / |     / |      |
     * (G)-|---(H) |      |
     *  | (A)---|-(B)     O----> (X)
     *  | /     | /      /
     *  |/      |/      /
     * (E)-----(F)     L (Z)
     */

    ecs::object a{"A"};
    ecs::object b{"B"};
    ecs::object c{"C"};
    ecs::object d{"D"};
    ecs::object e{"E"};
    ecs::object f{"F"};
    ecs::object g{"G"};
    ecs::object h{"H"};

    a.ensure<mass>()->val = 1.F;
    b.ensure<mass>()->val = 1.F;
    c.ensure<mass>()->val = 1.F;
    d.ensure<mass>()->val = 1.F;
    e.ensure<mass>()->val = 1.F;
    f.ensure<mass>()->val = 1.F;
    g.ensure<mass>()->val = 1.F;
    h.ensure<mass>()->val = 1.F;

    a.ensure<position>()->vec = {-1.F, -1.F, -1.F};
    b.ensure<position>()->vec = {+1.F, -1.F, -1.F};
    c.ensure<position>()->vec = {-1.F, +1.F, -1.F};
    d.ensure<position>()->vec = {+1.F, +1.F, -1.F};
    e.ensure<position>()->vec = {-1.F, -1.F, +1.F};
    f.ensure<position>()->vec = {+1.F, -1.F, +1.F};
    g.ensure<position>()->vec = {-1.F, +1.F, +1.F};
    h.ensure<position>()->vec = {+1.F, +1.F, +1.F};

    /*
    mgr.select<attraction, position>().cross<position>();

    physics p{mgr};
    p.apply_force();
    p.apply_acceleration();
    p.apply_velocity();
    */

    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}

