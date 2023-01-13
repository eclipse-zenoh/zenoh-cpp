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

#include "zenohcpp.h"

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

    KeyExprView unchecked(KeyExprUnchecked("*//*"));
    assert(unchecked.check());
    assert(!keyexpr_is_canon(unchecked.as_string_view()));
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
    KeyExprView foov("FOO");
    auto foobar = foov.concat("BAR");
    assert(foobar == "FOOBAR");

    KeyExpr foo("FOO");
    auto foobar1 = foo.concat("BAR");
    assert(foobar1 == "FOOBAR");
}

void join() {
    KeyExprView foov("FOO");
    auto foobar = foov.concat("BAR");
    assert(foobar == "FOO/BAR");

    KeyExpr foo("FOO");
    auto foobar1 = foo.concat("BAR");
    assert(foobar1 == "FOO/BAR");
}

void equals() {
    KeyExpr foo("FOO");
    KeyExprView foov("FOO");
    KeyExpr bar("BAR");
    KeyExprView barv("BAR");

    assert(foo.equals(foo));
    assert(foo.equals(foov));
    assert(foov.equals(foo));
    assert(foov.equals(foov));

    assert(!foo.equals(bar));
    assert(!foo.equals(barv));
    assert(!foov.equals(bar));
    assert(!foov.equals(barv));
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
    assert(err != 0);

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
    assert(err != 0);
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

int main(int argc, char** argv) {
    key_expr_view();
    key_expr();
    canonize();
    concat();
    equals();
    includes();
    intersects();
};
