//
// Copyright (c) 2024 ZettaScale Technology
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


void key_expr() {
    KeyExpr nul(nullptr);
    assert(!nul);

    KeyExpr foo("FOO");
    assert(static_cast<bool>(foo));
    assert(foo.as_string_view() == "FOO");
}

void canonize() {
    ZError err = 0;
    bool res;
    auto non_canon = "a/**/**/c";
    auto canon = "a/**/c";

    assert(KeyExpr::is_canon(canon));
    assert(!KeyExpr::is_canon(non_canon));

    // do not force canoniozation on keyexpr construction
    KeyExpr k_err(non_canon, false, &err);
#ifdef ZENOHCXX_ZENOHC //Pico does not validate key expressions yet.
    assert(!k_err);
    assert(err < 0);
#endif

    // enforce canonization
    KeyExpr k_ok(non_canon, true, &err);
    assert(static_cast<bool>(k_ok));
    assert(err == 0);
    assert(k_ok.as_string_view() == canon);
}

void concat() {
    KeyExpr foo("FOO");
    auto foobar1 = foo.concat("BAR");
    assert(foobar1.as_string_view() == "FOOBAR");
}

void join() {
    KeyExpr foo("FOO");
    auto foobar = foo.join(KeyExpr("BAR"));
    assert(foobar.as_string_view() == "FOO/BAR");
}

void equals() {
    KeyExpr nul(nullptr);

    KeyExpr foo("FOO");
    KeyExpr foo2("FOO");
    KeyExpr bar("BAR");

    assert(foo != bar);
    assert(foo == foo);
    assert(foo == foo2);
    assert(foo == "FOO");
    assert(foo != "BAR");
}

void includes() {
    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");
    assert(foostar.includes(foobar));
    assert(!foobar.includes(foostar));
}

void intersects() {
    KeyExpr foostar("FOO/*");
    KeyExpr foobar("FOO/BAR");
    KeyExpr starbuz("*/BUZ");
    KeyExpr foobuz("FOO/BUZ");

    assert(foostar.intersects(foobar));
    assert(!starbuz.intersects(foobar));
    assert(foobuz.intersects(starbuz));
    assert(starbuz.intersects(foobuz));
}

void declare(Session& s) {
    KeyExpr foobar("FOO/BAR");
    KeyExpr foostar("FOO/*");
    auto declared = s.declare_keyexpr(foobar);
    assert(static_cast<bool>(declared));

    assert(declared.as_string_view() == "FOO/BAR");
    assert(declared == foobar);
    assert(foostar.includes(declared));
    assert(declared.intersects(foobar));
    s.undeclare_keyexpr(std::move(declared));
    assert(!declared);
}

int main(int argc, char** argv) {
    key_expr();
    canonize();
    concat();
    join();
    equals();
    includes();
    intersects();

#ifdef ZENOHCXX_ZENOHC
    // Session based tests
    Config config = Config::create_default();
    auto session = Session::open(std::move(config));
    declare(session);
#endif
}
