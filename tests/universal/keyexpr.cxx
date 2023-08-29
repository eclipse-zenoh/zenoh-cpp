//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
#include "zenoh.hxx"
using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void key_expr_view() {
    KeyExprView nul(nullptr);
    assert(!nul.check());
    KeyExprView nulstr((const char*)nullptr);
    assert(!nulstr.check());

    KeyExprView foo("FOO");
    assert(foo.check());
    assert(foo == "FOO");
    assert(foo.as_bytes() == "FOO");
    assert(foo.as_string_view() == "FOO");

    std::string sfoo("FOO");
    KeyExprView ksfoo(sfoo);
    assert(ksfoo.check());
    assert(ksfoo == "FOO");
    assert(ksfoo.as_bytes() == "FOO");
    assert(ksfoo.as_string_view() == "FOO");

#ifdef ZENOHCXX_ZENOHC
    std::string_view svfoo("FOOBAR", 3);
    KeyExprView ksvfoo(svfoo);
    assert(ksvfoo.check());
    assert(ksvfoo == "FOO");
    assert(ksvfoo.as_bytes() == "FOO");
    assert(ksvfoo.as_string_view() == "FOO");

    KeyExprView unchecked("a/*", KeyExprUnchecked());
    assert(unchecked.check());
    assert(unchecked.as_string_view() == "a/*");

    std::string_view sunchecked("a/*//*", 3);
    KeyExprView svunchecked(sunchecked, KeyExprUnchecked());
    assert(svunchecked.as_string_view() == "a/*");
#endif
}

void key_expr() {
    KeyExpr nul(nullptr);
    assert(!nul.check());
    KeyExpr nulstr((const char*)nullptr);
    assert(!nulstr.check());

    KeyExpr foo("FOO");
    assert(foo.check());
    assert(foo == "FOO");
    assert(foo.as_bytes() == "FOO");
    assert(foo.as_string_view() == "FOO");
    assert(foo.as_keyexpr_view() == "FOO");
}

void canonize() {
    ErrNo err;
    bool res;
    auto non_canon = "a/**/**/c";
    auto canon = "a/**/c";

    assert(keyexpr_is_canon(canon));
    assert(!keyexpr_is_canon(non_canon));
    assert(keyexpr_is_canon(canon, err));
    assert(err == 0);
    assert(!keyexpr_is_canon(non_canon, err));
    assert(err != 0);

    std::string foo(non_canon);
    res = keyexpr_canonize(foo, err);
    assert(foo == canon);
    assert(err == 0);
    assert(res);
}

void concat() {
#ifdef ZENOHCXX_ZENOHC
    KeyExprView foov("FOO");
    auto foobar = foov.concat("BAR");
    assert(foobar == "FOOBAR");

    KeyExpr foo("FOO");
    auto foobar1 = foo.concat("BAR");
    assert(foobar1 == "FOOBAR");

    assert(keyexpr_concat("FOO", "BAR") == "FOOBAR");
    assert(keyexpr_concat(KeyExpr("FOO"), "BAR") == "FOOBAR");
    assert(keyexpr_concat(KeyExprView("FOO"), "BAR") == "FOOBAR");
#endif
}

void join() {
#ifdef ZENOHCXX_ZENOHC
    KeyExprView foov("FOO");
    auto foobar = foov.concat("BAR");
    assert(foobar == "FOO/BAR");

    KeyExpr foo("FOO");
    auto foobar1 = foo.concat("BAR");
    assert(foobar1 == "FOO/BAR");

    assert(keyexpr_join("FOO", "BAR") == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), "BAR") == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), "BAR") == "FOO/BAR");
    assert(keyexpr_join("FOO", KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), KeyExpr("BAR")) == "FOO/BAR");
    assert(keyexpr_join("FOO", KeyExprView("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExpr("FOO"), KeyExprView("BAR")) == "FOO/BAR");
    assert(keyexpr_join(KeyExprView("FOO"), KeyExprView("BAR")) == "FOO/BAR");
#endif
}

void equals() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExpr foo("FOO");
    KeyExprView foov("FOO");
    KeyExpr bar("BAR");
    KeyExprView barv("BAR");

#ifdef ZENOHCXX_ZENOHC
    assert(foo.equals(foo));
    assert(foo.equals(foov));
    assert(foov.equals(foo));
    assert(foov.equals(foov));
    assert(keyexpr_equals("FOO", "FOO"));
    assert(!keyexpr_equals("FOO", "BAR"));
    assert(!keyexpr_equals("FOO", nul));
    assert(!foo.equals(bar));
    assert(!foo.equals(barv));
    assert(!foov.equals(bar));
    assert(!foov.equals(barv));
    assert(!foo.equals(nul));
    assert(!foov.equals(nul));
#endif
    assert(foo.equals(foo, err));
    assert(err == 0);
    assert(foo.equals(foov, err));
    assert(err == 0);
    assert(foov.equals(foo, err));
    assert(err == 0);
    assert(foov.equals(foov, err));
    assert(err == 0);

    assert(keyexpr_equals("FOO", "FOO", err));
    assert(err == 0);

    assert(!foo.equals(bar, err));
    assert(err == 0);
    assert(!foo.equals(barv, err));
    assert(err == 0);
    assert(!foov.equals(bar, err));
    assert(err == 0);
    assert(!foov.equals(barv, err));
    assert(err == 0);

    assert(!keyexpr_equals("FOO", "BAR", err));
    assert(err == 0);

    assert(!foo.equals(nul, err));
    assert(err < 0);
    assert(!foov.equals(nul, err));
    assert(err < 0);

    assert(!keyexpr_equals("FOO", nul, err));
    assert(err < 0);
}

void includes() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");

#ifdef ZENOHCXX_ZENOHC
    assert(foostarv.includes(foobarv));
    assert(!foobarv.includes(foostarv));
    assert(!foostarv.includes(nul));
    assert(keyexpr_includes("FOO/*", "FOO/BAR"));
#endif

    assert(foostarv.includes(foobarv, err));
    assert(err == 0);
    assert(!foobarv.includes(foostarv, err));
    assert(err == 0);
    assert(!foostarv.includes(nul, err));
    assert(err < 0);
    assert(keyexpr_includes("FOO/*", "FOO/BAR", err));
    assert(err == 0);

    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");

#ifdef ZENOHCXX_ZENOHC
    assert(foostar.includes(foobar));
    assert(!foobar.includes(foostar));
    assert(!foostar.includes(nul));
    assert(!keyexpr_includes("FOO/BAR", "FOO/*"));
    assert(!keyexpr_includes("FOO/*", nul));
#endif

    assert(foostar.includes(foobar, err));
    assert(err == 0);
    assert(!foobar.includes(foostar, err));
    assert(err == 0);
    assert(!foostar.includes(nul, err));
    assert(err < 0);
    assert(!keyexpr_includes("FOO/BAR", "FOO/*", err));
    assert(err == 0);
    assert(!keyexpr_includes("FOO/*", nul, err));
    assert(err < 0);

    KeyExpr foo("FOO");
#ifdef ZENOHCXX_ZENOHC
    assert(!keyexpr_includes(foo, "FOO/BAR"));
    assert(!keyexpr_includes("FOO/BAR", foo));
#endif
    assert(!keyexpr_includes(foo, "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_includes("FOO/BAR", foo, err));
    assert(err == 0);
}

void intersects() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");
    KeyExprView starbuzv("*/BUZ");

#ifdef ZENOHCXX_ZENOHC
    assert(foostarv.intersects(foobarv));
    assert(!starbuzv.intersects(foobarv));
    assert(!foostarv.intersects(nul));
    assert(keyexpr_intersects("FOO/*", "FOO/BAR"));
    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR"));
    assert(!keyexpr_intersects("FOO/*", nul));
#endif

    assert(foostarv.intersects(foobarv, err));
    assert(err == 0);
    assert(!starbuzv.intersects(foobarv, err));
    assert(err == 0);
    assert(!foostarv.intersects(nul, err));
    assert(err != 0);

    assert(keyexpr_intersects("FOO/*", "FOO/BAR", err));
    assert(err == 0);

    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR", err));
    assert(err == 0);

    assert(!keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);

    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");
    KeyExpr starbuz("*/BUZ");

#ifdef ZENOHCXX_ZENOHC
    assert(foostar.intersects(foobar));
    assert(!starbuz.intersects(foobar));
    assert(!foostar.intersects(nul));
    assert(keyexpr_intersects("FOO/*", foobar));
    assert(!keyexpr_intersects("*/BUZ", foobar));
    assert(!keyexpr_intersects("FOO/*", nul));
#endif

    assert(foostar.intersects(foobar, err));
    assert(err == 0);
    assert(!starbuz.intersects(foobar, err));
    assert(err == 0);
    assert(!foostar.intersects(nul, err));
    assert(err != 0);

    assert(keyexpr_intersects("FOO/*", foobar, err));
    assert(err == 0);
    assert(!keyexpr_intersects("*/BUZ", foobar, err));
    assert(err == 0);
    assert(!keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);
}

#include <variant>

void undeclare(Session& s) {
    auto keyexpr = s.declare_keyexpr("foo/bar");
    assert(keyexpr.check());
    ErrNo err;
    assert(s.undeclare_keyexpr(std::move(keyexpr), err));
    assert(err == 0);
    assert(!keyexpr.check());
}

void equals_declared(Session& s) {
    KeyExprView nul(nullptr);
    ErrNo err;

    auto foo = s.declare_keyexpr("FOO");
    auto bar = s.declare_keyexpr("BAR");
    KeyExprView foov("FOO");
    KeyExprView barv("BAR");

    assert(foo.check());
    assert(bar.check());
    assert(foov.check());
    assert(barv.check());

#ifdef ZENOHCXX_ZENOHC
    // zenoh-c is able to compare declared keyexprs
    assert(foo.equals(foo, err));
    assert(err == 0);
    assert(foo.equals(foov, err));
    assert(err == 0);
    assert(!foo.equals(bar, err));
    assert(err == 0);
    assert(!foo.equals(barv, err));
    assert(err == 0);
#else
    // zenoh-pico returns error when comapring declared keyexprs: the string value is avaliable in session only
    assert(!foo.equals(foo, err));
    assert(err < 0);
    assert(!foo.equals(foov, err));
    assert(err < 0);
    assert(!foo.equals(bar, err));
    assert(err < 0);
    assert(!foo.equals(barv, err));
    assert(err < 0);
#endif

    // both zenoh-c and zenoh-pico are able to compare declared keyexprs through the session
    assert(s.keyexpr_equals(foo, foo));
    assert(s.keyexpr_equals(foo, foov));
    assert(!s.keyexpr_equals(foo, bar));
    assert(!s.keyexpr_equals(foo, barv));

    assert(s.keyexpr_equals(foo, foo, err));
    assert(err == 0);
    assert(s.keyexpr_equals(foo, foov, err));
    assert(err == 0);
    assert(!s.keyexpr_equals(foo, bar, err));
    assert(err == 0);
    assert(!s.keyexpr_equals(foo, barv, err));
    assert(err == 0);
}

void includes_declared(Session& s) {
    KeyExprView nul(nullptr);
    ErrNo err;

    auto foostar = s.declare_keyexpr("FOO/*");
    auto foobar = s.declare_keyexpr("FOO/BAR");
    auto starbuz = s.declare_keyexpr("*/BUZ");
    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");
    KeyExprView starbuzv("*/BUZ");

    assert(foostar.check());
    assert(foobar.check());
    assert(starbuz.check());
    assert(foostarv.check());
    assert(foobarv.check());
    assert(starbuzv.check());

#ifdef ZENOHCXX_ZENOHC
    // zenoh-c is able to check declared keyexprs
    assert(foostar.includes(foobar, err));
    assert(err == 0);
    assert(!starbuz.includes(foobar, err));
    assert(err == 0);
    assert(!foostar.includes(nul, err));
    assert(err < 0);
    assert(keyexpr_includes("FOO/*", "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_includes("*/BUZ", "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_includes("FOO/*", nul, err));
    assert(err < 0);
#else
    // zenoh-pico returns error when checking declared keyexprs: the string value is avaliable in session only
    assert(!foostar.includes(foobar, err));
    assert(err < 0);
    assert(!starbuz.includes(foobar, err));
    assert(err < 0);
    assert(!foostar.includes(nul, err));
    assert(err < 0);
    assert(!keyexpr_includes("FOO/*", "FOO/BAR", err));
    assert(err < 0);
    assert(!keyexpr_includes("*/BUZ", "FOO/BAR", err));
    assert(err < 0);
    assert(!keyexpr_includes("FOO/*", nul, err));
    assert(err < 0);
#endif

    // both zenoh-c and zenoh-pico are able to check declared keyexprs through the session
    assert(s.keyexpr_includes(foostar, foobar));
    assert(!s.keyexpr_includes(starbuz, foobar));
    assert(!s.keyexpr_includes(foostar, nul));
    assert(s.keyexpr_includes("FOO/*", "FOO/BAR"));
    assert(!s.keyexpr_includes("*/BUZ", "FOO/BAR"));
    assert(!s.keyexpr_includes("FOO/*", nul));

    assert(s.keyexpr_includes(foostar, foobar, err));
    assert(err == 0);
    assert(!s.keyexpr_includes(starbuz, foobar, err));
    assert(err == 0);
    assert(!s.keyexpr_includes(foostar, nul, err));
    assert(err < 0);
    assert(s.keyexpr_includes("FOO/*", "FOO/BAR", err));
    assert(err == 0);
    assert(!s.keyexpr_includes("*/BUZ", "FOO/BAR", err));
    assert(err == 0);
    assert(!s.keyexpr_includes("FOO/*", nul, err));
    assert(err < 0);
}

void intersects_declared(Session& s) {
    KeyExprView nul(nullptr);
    ErrNo err;

    auto foostar = s.declare_keyexpr("FOO/*");
    auto foobar = s.declare_keyexpr("FOO/BAR");
    auto starbuz = s.declare_keyexpr("*/BUZ");
    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");
    KeyExprView starbuzv("*/BUZ");

    assert(foostar.check());
    assert(foobar.check());
    assert(starbuz.check());
    assert(foostarv.check());
    assert(foobarv.check());
    assert(starbuzv.check());

#ifdef ZENOHCXX_ZENOHC
    // zenoh-c is able to check declared keyexprs
    assert(foostar.intersects(foobar, err));
    assert(err == 0);
    assert(!starbuz.intersects(foobar, err));
    assert(err == 0);
    assert(!foostar.intersects(nul, err));
    assert(err < 0);
    assert(keyexpr_intersects("FOO/*", "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR", err));
    assert(err == 0);
    assert(!keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);
#else
    // zenoh-pico returns error when checking declared keyexprs: the string value is avaliable in session only
    assert(!foostar.intersects(foobar, err));
    assert(err < 0);
    assert(!starbuz.intersects(foobar, err));
    assert(err < 0);
    assert(!foostar.intersects(nul, err));
    assert(err < 0);
    assert(!keyexpr_intersects("FOO/*", "FOO/BAR", err));
    assert(err < 0);
    assert(!keyexpr_intersects("*/BUZ", "FOO/BAR", err));
    assert(err < 0);
    assert(!keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);
#endif

    // both zenoh-c and zenoh-pico are able to check declared keyexprs through the session
    assert(s.keyexpr_intersects(foostar, foobar));
    assert(!s.keyexpr_intersects(starbuz, foobar));
    assert(!s.keyexpr_intersects(foostar, nul));
    assert(s.keyexpr_intersects("FOO/*", "FOO/BAR"));
    assert(!s.keyexpr_intersects("*/BUZ", "FOO/BAR"));
    assert(!s.keyexpr_intersects("FOO/*", nul));

    assert(s.keyexpr_intersects(foostar, foobar, err));
    assert(err == 0);
    assert(!s.keyexpr_intersects(starbuz, foobar, err));
    assert(err == 0);
    assert(!s.keyexpr_intersects(foostar, nul, err));
    assert(err < 0);
    assert(s.keyexpr_intersects("FOO/*", "FOO/BAR", err));
    assert(err == 0);
    assert(!s.keyexpr_intersects("*/BUZ", "FOO/BAR", err));
    assert(err == 0);
    assert(!s.keyexpr_intersects("FOO/*", nul, err));
    assert(err < 0);
}

int main(int argc, char** argv) {
    key_expr_view();
    key_expr();
    canonize();
    concat();
    equals();
    includes();
    intersects();
    // Session based tests
    Config config;
    auto session = open(std::move(config));
    if (auto psession = std::get_if<Session>(&session)) {
        undeclare(*psession);
        equals_declared(*psession);
        includes_declared(*psession);
    } else {
        auto error = std::get<ErrorMessage>(session);
        std::cerr << "Error: " << error.as_string_view() << std::endl;
        std::cerr << "For zenog-pico make sure that the zenoh router is running" << std::endl;
    }
}
