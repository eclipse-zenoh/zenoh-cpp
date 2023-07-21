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

// Pretty print type name
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
#include <string_view>
template <class T>
constexpr std::string_view type_name() {
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#else
    return string_view(p.data() + 49, p.find(';', 49) - 49);
#endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

#include "zenoh.hxx"
using namespace zenoh;

#undef NDEBUG
#include <assert.h>

//
// Test for all variants of construnctng 'ClosureMoveParam' closures
//
// - from function
// - from object copy
// - from object reference
// - from object rvalue reference
// - from lambda expression
//

size_t callcnt = 1;
size_t dropcnt = 1;

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

void on_reply_2(Reply&&) { callcnt *= 2; };
void on_reply_3(Reply&&) { callcnt *= 3; };
void on_reply_5(Reply&&) { callcnt *= 5; };
void on_reply_7(Reply&&) { callcnt *= 7; };
void on_reply_11(Reply&&) { callcnt *= 11; };

struct OnCall {
    OnCall() = default;
    OnCall(const OnCall&) = delete;
    OnCall(OnCall&&) = default;
    OnCall& operator=(const OnCall&) = delete;
    OnCall& operator=(OnCall&&) = default;

    OnCall(int _v) : v(_v) {}
    void operator()(Reply&&) { callcnt *= v; };
    int v;
};

void test_call() {
    std::cout << "test_call" << std::endl;

    ClosureReply f(on_reply_3);
    ClosureReply o(OnCall(5));
    OnCall o7(7);
    ClosureReply r(o7);
    OnCall o11(11);
    ClosureReply m(std::move(o11));
    ClosureReply l([](Reply&&) { callcnt *= 13; });

    // rvalue parameter tests
    callcnt = 1;
    f(Reply(nullptr));
    o(Reply(nullptr));
    r(Reply(nullptr));
    m(Reply(nullptr));
    l(Reply(nullptr));

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
    std::cout << "test_call_f_drop" << std::endl;

    callcnt = 1;
    dropcnt = 1;
    {
        ClosureReply ff(on_reply_2, on_drop_2);
        ClosureReply fo(on_reply_3, OnDrop(3));
        OnDrop d5(5);
        ClosureReply fr(on_reply_5, d5);
        OnDrop d7(7);
        ClosureReply fm(on_reply_7, std::move(d7));
        ClosureReply fl(on_reply_11, []() { dropcnt *= 11; });

        ff(Reply(nullptr));
        fo(Reply(nullptr));
        fr(Reply(nullptr));
        fm(Reply(nullptr));
        fl(Reply(nullptr));

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }

    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_o_drop() {
    std::cout << "test_call_o_drop" << std::endl;

    callcnt = 1;
    dropcnt = 1;
    {
        ClosureReply of(OnCall(2), on_drop_2);
        ClosureReply oo(OnCall(3), OnDrop(3));
        OnDrop d5(5);
        ClosureReply or_(OnCall(5), d5);
        OnDrop d7(7);
        ClosureReply om(OnCall(7), std::move(d7));
        ClosureReply ol(OnCall(11), []() { dropcnt *= 11; });

        of(Reply(nullptr));
        oo(Reply(nullptr));
        or_(Reply(nullptr));
        om(Reply(nullptr));
        ol(Reply(nullptr));

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }

    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_r_drop() {
    std::cout << "test_call_r_drop" << std::endl;

    callcnt = 1;
    dropcnt = 1;
    {
        OnCall f2(2);
        OnCall f3(3);
        OnCall f5(5);
        OnCall f7(7);
        OnCall f11(11);

        ClosureReply rf(f2, on_drop_2);
        ClosureReply ro(f3, OnDrop(3));
        OnDrop d5(5);
        ClosureReply rr(f5, d5);
        OnDrop d7(7);
        ClosureReply rm(f7, std::move(d7));
        ClosureReply rl(f11, []() { dropcnt *= 11; });

        rf(Reply(nullptr));
        ro(Reply(nullptr));
        rr(Reply(nullptr));
        rm(Reply(nullptr));
        rl(Reply(nullptr));

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }
    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_m_drop() {
    std::cout << "test_call_m_drop" << std::endl;

    callcnt = 1;
    dropcnt = 1;
    {
        OnCall f2(2);
        OnCall f3(3);
        OnCall f5(5);
        OnCall f7(7);
        OnCall f11(11);

        ClosureReply mf(std::move(f2), on_drop_2);
        ClosureReply mo(std::move(f3), OnDrop(3));
        OnDrop d5(5);
        ClosureReply mr(std::move(f5), d5);
        OnDrop d7(7);
        ClosureReply mm(std::move(f7), std::move(d7));
        ClosureReply ml(std::move(f11), []() { dropcnt *= 11; });

        mf(Reply(nullptr));
        mo(Reply(nullptr));
        mr(Reply(nullptr));
        mm(Reply(nullptr));
        ml(Reply(nullptr));

        assert(dropcnt == size_t(1));
        assert(callcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
    }
    assert(dropcnt == size_t(1) * 2 * 3 * 5 * 7 * 11);
}

void test_call_l_drop() {
    std::cout << "test_call_l_drop" << std::endl;

    callcnt = 1;
    dropcnt = 1;
    {
        ClosureReply lf([](Reply&&) { callcnt *= 2; }, on_drop_2);
        ClosureReply lo([](Reply&&) { callcnt *= 3; }, OnDrop(3));
        OnDrop d5(5);
        ClosureReply lr([](Reply&&) { callcnt *= 5; }, d5);
        OnDrop d7(7);
        ClosureReply lm([](Reply&&) { callcnt *= 7; }, std::move(d7));
        ClosureReply ll([](Reply&&) { callcnt *= 11; }, []() { dropcnt *= 11; });

        lf(Reply(nullptr));
        lo(Reply(nullptr));
        lr(Reply(nullptr));
        lm(Reply(nullptr));
        ll(Reply(nullptr));

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
