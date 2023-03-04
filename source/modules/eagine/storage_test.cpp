/// @file
///
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_ctx.hpp>
import eagine.core;
import eagine.ecs;
import std;
//------------------------------------------------------------------------------
// storage capabilities
//------------------------------------------------------------------------------
void storage_caps_all(auto& s) {
    eagitest::case_ test{s, 1, "all capabilities"};
    eagitest::track trck{test, 0, 1};

    const eagine::ecs::storage_caps all_attrs{eagine::ecs::all_storage_caps()};

    eagine::ecs::storage_caps test_attrs;

    for(const auto& info :
        eagine::enumerators<eagine::ecs::storage_cap_bit>()) {
        test.check(all_attrs.has(info.enumerator), "has enumerator");
        test.check(test_attrs.has_not(info.enumerator), "has not enumerator");
        test_attrs.set(info.enumerator);
        trck.checkpoint(1);
    }

    test.check(all_attrs == test_attrs, "all set");
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::ctx_suite test{ctx, "storage", 1};
    test.once(storage_caps_all);
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_ctx.hpp>
