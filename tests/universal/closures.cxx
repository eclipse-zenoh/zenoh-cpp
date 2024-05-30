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
#include <iostream>

void test_call_drop() {
    size_t calls_count = 0;
    size_t drop_count = 0;

    auto on_call = [&calls_count] {
        calls_count++;
        return calls_count;
    };
    using OnCall = decltype(on_call);
    auto on_drop = [&drop_count] {
        drop_count++;
    };

    using OnDrop = decltype(on_drop);
    
    auto c = detail::closures::Closure<OnCall, OnDrop, size_t>(on_call, on_drop);
    c.call();
    c.call();
    c.drop();

    assert(calls_count = 2);
    assert(drop_count == 1);
}

void test_context() {
    size_t calls_count = 0;
    bool dropped = 0;

    auto on_call = [&calls_count] (size_t c) {
        calls_count += c;
        return calls_count;
    };
    using OnCall = decltype(on_call);
    auto on_drop = [&dropped] {
        dropped = true;
    };

    using OnDrop = decltype(on_drop);
    
    auto context = detail::closures::Closure<OnCall, OnDrop, size_t, size_t>::into_context(on_call, on_drop);
    detail::closures::IClosure<size_t, size_t>::call_from_context(context, 2);
    detail::closures::IDroppable::delete_from_context(context);

    assert(calls_count = 3);
    assert(dropped);
}

int main(int argc, char** argv) {
    test_call_drop();
    test_context();
}