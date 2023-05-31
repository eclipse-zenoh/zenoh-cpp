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
#endif
}

void equals() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExpr foo("FOO");
    KeyExprView foov("FOO");
    KeyExpr bar("BAR");
    KeyExprView barv("BAR");

    assert(foo.equals(foo));
    assert(foo.equals(foo, err));
    assert(err == 0);
    assert(foo.equals(foov));
    assert(foo.equals(foov, err));
    assert(err == 0);
    assert(foov.equals(foo));
    assert(foov.equals(foo, err));
    assert(err == 0);
    assert(foov.equals(foov));
    assert(foov.equals(foov, err));
    assert(err == 0);

    assert(!foo.equals(bar));
    assert(!foo.equals(bar, err));
    assert(err == 0);
    assert(!foo.equals(barv));
    assert(!foo.equals(barv, err));
    assert(err == 0);
    assert(!foov.equals(bar));
    assert(!foov.equals(bar, err));
    assert(err == 0);
    assert(!foov.equals(barv));
    assert(!foov.equals(barv, err));
    assert(err == 0);

    assert(!foo.equals(nul));
    assert(!foo.equals(nul, err));
    assert(err < 0);

    assert(!foov.equals(nul));
    assert(!foov.equals(nul, err));
    assert(err < 0);
}

void includes() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");
    assert(foostarv.includes(foobarv));
    assert(foostarv.includes(foobarv, err));
    assert(err == 0);
    assert(!foobarv.includes(foostarv));
    assert(!foobarv.includes(foostarv, err));
    assert(err == 0);
    assert(!foostarv.includes(nul));
    assert(!foostarv.includes(nul, err));
    assert(err < 0);

    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");
    assert(foostar.includes(foobar));
    assert(foostar.includes(foobar, err));
    assert(err == 0);
    assert(!foobar.includes(foostar));
    assert(!foobar.includes(foostar, err));
    assert(err == 0);
    assert(!foostar.includes(nul));
    assert(!foostar.includes(nul, err));
    assert(err < 0);
}

void intersects() {
    KeyExprView nul(nullptr);
    ErrNo err;

    KeyExprView foostarv("FOO/*");
    KeyExprView foobarv("FOO/BAR");
    KeyExprView starbuzv("*/BUZ");
    assert(foostarv.intersects(foobarv));
    assert(!starbuzv.intersects(foobarv));
    assert(!foostarv.intersects(nul));
    assert(foostarv.intersects(foobarv, err));
    assert(err == 0);
    assert(!starbuzv.intersects(foobarv, err));
    assert(err == 0);
    assert(!foostarv.intersects(nul, err));
    assert(err != 0);

    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");
    KeyExpr starbuz("*/BUZ");
    assert(foostar.intersects(foobar));
    assert(!starbuz.intersects(foobar));
    assert(!foostar.intersects(nul));
    assert(foostar.intersects(foobar, err));
    assert(err == 0);
    assert(!starbuz.intersects(foobar, err));
    assert(err == 0);
    assert(!foostar.intersects(nul, err));
    assert(err != 0);
}

#include <variant>

void undeclare() {
    Config config;
    auto session = open(std::move(config));
    if (auto psession = std::get_if<Session>(&session)) {
        auto keyexpr = psession->declare_keyexpr("foo/bar");
        assert(keyexpr.check());
        ErrNo err;
        assert(psession->undeclare_keyexpr(std::move(keyexpr), err));
        assert(err == 0);
        assert(!keyexpr.check());
    } else {
        auto error = std::get<ErrorMessage>(session);
        std::cerr << "Error: " << error.as_string_view() << std::endl;
// zenohpico is unable to open session without zenoh router started
#ifdef ZENOHCXX_ZENOHC
        assert(false);
#endif
    }
}

int main(int argc, char** argv) {
    key_expr_view();
    key_expr();
    canonize();
    concat();
    equals();
    includes();
    intersects();
    undeclare();
};
