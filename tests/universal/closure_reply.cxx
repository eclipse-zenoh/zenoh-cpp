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
// Test for all variants of construnctng 'ClosureMoveParam' closures
//
// - from function
// - from object copy
// - from object reference
// - from object rvalue reference
// - from lambda expression
//

size_t gcnt = 1;

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

void on_reply(Reply&&) { gcnt *= 2; };

struct OnReply {
    OnReply() = default;
    OnReply(const OnReply&) = delete;
    OnReply(OnReply&&) = default;
    OnReply& operator=(const OnReply&) = delete;
    OnReply& operator=(OnReply&&) = default;

    OnReply(int _v) : v(_v) {}
    void operator()(Reply&&) { gcnt *= v; };
    int v;
};

void test_constructors() {
    ClosureReply closure_reply_f(on_reply);
    ClosureReply closure_reply_obj(OnReply(5));
    OnReply on_reply_obj_ref(7);
    ClosureReply closure_reply_obj_ref(on_reply_obj_ref);
    OnReply on_reply_obj_moveref(11);
    ClosureReply closure_reply_obj_moveref(std::move(on_reply_obj_moveref));
    ClosureReply closure_reply_lambda([](Reply&&) { gcnt *= 13; });

    // rvalue parameter tests
    gcnt = 1;
    closure_reply_f(Reply(nullptr));
    closure_reply_obj(Reply(nullptr));
    closure_reply_obj_ref(Reply(nullptr));
    closure_reply_obj_moveref(Reply(nullptr));
    closure_reply_lambda(Reply(nullptr));

    show_primes(gcnt);
    assert(gcnt == size_t(1) * 2 * 5 * 7 * 11 * 13);
}

void test_add_call() {
    ClosureReply closure_reply_f;
    closure_reply_f.add_call(on_reply);
    ClosureReply closure_reply_obj;
    closure_reply_obj.add_call(OnReply(5));
    ClosureReply closure_reply_obj_ref;
    OnReply on_reply_obj_ref(7);
    closure_reply_obj_ref.add_call(on_reply_obj_ref);
    OnReply on_reply_obj_moveref(11);
    ClosureReply closure_reply_obj_moveref;
    closure_reply_obj_moveref.add_call(std::move(on_reply_obj_moveref));
    ClosureReply closure_reply_lambda;
    closure_reply_lambda.add_call([](Reply&&) { gcnt *= 13; });

    gcnt = 1;
    closure_reply_f(Reply(nullptr));
    closure_reply_obj(Reply(nullptr));
    closure_reply_obj_ref(Reply(nullptr));
    closure_reply_obj_moveref(Reply(nullptr));
    closure_reply_lambda(Reply(nullptr));

    show_primes(gcnt);
    assert(gcnt == size_t(1) * 2 * 5 * 7 * 11 * 13);

    gcnt = 1;
    closure_reply_f.add_call(OnReply(17));
    closure_reply_obj.add_call(OnReply(19));
    closure_reply_obj_ref.add_call(OnReply(23));
    closure_reply_obj_moveref.add_call(OnReply(29));
    closure_reply_lambda.add_call(OnReply(31));
    closure_reply_f(Reply(nullptr));
    closure_reply_obj(Reply(nullptr));
    closure_reply_obj_ref(Reply(nullptr));
    closure_reply_obj_moveref(Reply(nullptr));
    closure_reply_lambda(Reply(nullptr));

    show_primes(gcnt);
    assert(gcnt == size_t(1) * 2 * 5 * 7 * 11 * 13 * 17 * 19 * 23 * 29 * 31);
}

void on_drop_2() { gcnt *= 2; };
void on_drop_17() { gcnt *= 17; };

struct OnDrop {
    OnDrop(int _v) : v(_v) {}
    void operator()() { gcnt *= v; };
    int v;
};

void test_add_drop() {
    gcnt = 1;
    {
        ClosureReply closure_reply_f;
        ClosureReply closure_reply_obj;
        ClosureReply closure_reply_obj_ref;
        ClosureReply closure_reply_obj_moveref;
        ClosureReply closure_reply_lambda;

        closure_reply_f.add_drop(on_drop_2);
        closure_reply_obj.add_drop(OnDrop(5));
        OnDrop on_drop_obj_ref(7);
        closure_reply_obj_ref.add_drop(on_drop_obj_ref);
        OnDrop on_drop_obj_moveref(11);
        closure_reply_obj_moveref.add_drop(std::move(on_drop_obj_moveref));
        closure_reply_lambda.add_drop([]() { gcnt *= 13; });

        closure_reply_f.add_drop(on_drop_17);
        closure_reply_obj.add_drop(OnDrop(19));
        OnDrop on_drop_obj_ref_23(23);
        closure_reply_obj_ref.add_drop(on_drop_obj_ref_23);
        OnDrop on_drop_obj_moveref_29(29);
        closure_reply_obj_moveref.add_drop(std::move(on_drop_obj_moveref_29));
        closure_reply_lambda.add_drop([]() { gcnt *= 31; });

        assert(gcnt == size_t(1));
    }

    show_primes(gcnt);
    assert(gcnt == size_t(1) * 2 * 5 * 7 * 11 * 13 * 17 * 19 * 23 * 29 * 31);
}

int main(int argc, char** argv) {
    test_constructors();
    test_add_call();
    test_add_drop();
}
