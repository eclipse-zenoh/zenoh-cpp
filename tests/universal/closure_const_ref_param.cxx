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

//
// Test for all variants of construnctng 'ClosureConstRefParam' closures
//
// - from function
// - from object copy
// - from object reference
// - from object rvalue reference
// - from lambda expression
//

size_t callcnt = 1;
size_t dropcnt = 1;

Query query(::z_query_t{0});

void show_primes(size_t v) {
    std::cout << v << " = 1";
    size_t d = 2;
    while (v > 1) {
        if (v % d == 0) {
            std::cout << " * " << d;
            v /= d;
        } else {
            d++;
        }
    }
    std::cout << std::endl;
}

void on_reply_2(const Query&) { callcnt *= 2; };
void on_reply_3(const Query&) { callcnt *= 3; };
void on_reply_5(const Query&) { callcnt *= 5; };
void on_reply_7(const Query&) { callcnt *= 7; };
void on_reply_11(const Query&) { callcnt *= 11; };

struct OnCall {
    OnCall() = default;
    OnCall(const OnCall&) = delete;
    OnCall(OnCall&&) = default;
    OnCall& operator=(const OnCall&) = delete;
    OnCall& operator=(OnCall&&) = default;

    OnCall(int _v) : v(_v) {}
    void operator()(const Query&) { callcnt *= v; };
    int v;
};

void test_call() {
    ClosureQuery f(on_reply_3);
    ClosureQuery o(OnCall(5));
    OnCall o7(7);
    ClosureQuery r(o7);
    OnCall o11(11);
    ClosureQuery m(std::move(o11));
    ClosureQuery l([](const Query&) { callcnt *= 13; });

    // rvalue parameter tests
    callcnt = 1;
    f(query);
    o(query);
    r(query);
    m(query);
    l(query);

    assert(callcnt == size_t(1) * 3 * 5 * 7 * 11 * 13);
}

void on_drop_2() { dropcnt *= 2; };
void on_drop_3() { dropcnt *= 3; };
void on_drop_5() { dropcnt *= 5; };

void on_drop_17() { dropcnt *= 17; };

struct OnDrop {
    OnDrop(int _v) : v(_v) {}
    void operator()() { dropcnt *= v; };
    int v;
};

void test_call_f_drop() {
    callcnt = 1;
    dropcnt = 1;
    {
        ClosureQuery ff(on_reply_2, on_drop_2);
        ClosureQuery fo(on_reply_3, OnDrop(3));
        OnDrop d5(5);
        ClosureQuery fr(on_reply_5, d5);
        OnDrop d7(7);
        ClosureQuery fm(on_reply_7, std::move(d7));
        ClosureQuery fl(on_reply_11, []() { dropcnt *= 11; });

        ff(query);
        fo(query);
        fr(query);
        fm(query);
        fl(query);

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }

    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_o_drop() {
    callcnt = 1;
    dropcnt = 1;
    {
        ClosureQuery of(OnCall(2), on_drop_2);
        ClosureQuery oo(OnCall(3), OnDrop(3));
        OnDrop d5(5);
        ClosureQuery or_(OnCall(5), d5);
        OnDrop d7(7);
        ClosureQuery om(OnCall(7), std::move(d7));
        ClosureQuery ol(OnCall(11), []() { dropcnt *= 11; });

        of(query);
        oo(query);
        or_(query);
        om(query);
        ol(query);

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }

    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_r_drop() {
    callcnt = 1;
    dropcnt = 1;
    {
        OnCall f2(2);
        OnCall f3(3);
        OnCall f5(5);
        OnCall f7(7);
        OnCall f11(11);

        ClosureQuery rf(f2, on_drop_2);
        ClosureQuery ro(f3, OnDrop(3));
        OnDrop d5(5);
        ClosureQuery rr(f5, d5);
        OnDrop d7(7);
        ClosureQuery rm(f7, std::move(d7));
        ClosureQuery rl(f11, []() { dropcnt *= 11; });

        rf(query);
        ro(query);
        rr(query);
        rm(query);
        rl(query);

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }
    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_m_drop() {
    callcnt = 1;
    dropcnt = 1;
    {
        OnCall f2(2);
        OnCall f3(3);
        OnCall f5(5);
        OnCall f7(7);
        OnCall f11(11);

        ClosureQuery mf(std::move(f2), on_drop_2);
        ClosureQuery mo(std::move(f3), OnDrop(3));
        OnDrop d5(5);
        ClosureQuery mr(std::move(f5), d5);
        OnDrop d7(7);
        ClosureQuery mm(std::move(f7), std::move(d7));
        ClosureQuery ml(std::move(f11), []() { dropcnt *= 11; });

        mf(query);
        mo(query);
        mr(query);
        mm(query);
        ml(query);

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }
    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_l_drop() {
    callcnt = 1;
    dropcnt = 1;
    {
        ClosureQuery lf([](const Query&) { callcnt *= 2; }, on_drop_2);
        ClosureQuery lo([](const Query&) { callcnt *= 3; }, OnDrop(3));
        OnDrop d5(5);
        ClosureQuery lr([](const Query&) { callcnt *= 5; }, d5);
        OnDrop d7(7);
        ClosureQuery lm([](const Query&) { callcnt *= 7; }, std::move(d7));
        ClosureQuery ll([](const Query&) { callcnt *= 11; }, []() { dropcnt *= 11; });

        lf(query);
        lo(query);
        lr(query);
        lm(query);
        ll(query);

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }
    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

int main(int argc, char** argv) {
    test_call();
    test_call_f_drop();
    test_call_o_drop();
    test_call_r_drop();
    test_call_m_drop();
    test_call_l_drop();
}
