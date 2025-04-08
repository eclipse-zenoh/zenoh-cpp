//
// Copyright (c) 2025 ZettaScale Technology
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

#include <chrono>
#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

void test_multiple_batching() {
    auto session1 = Session::open(Config::create_default());
    auto batch_guard = session1.start_batching();

    ZResult err = Z_OK;
    auto batch_guard2 = session1.start_batching(&err);
    assert(err != Z_OK);
}

void test_batching() {
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    auto subscriber = session2.declare_subscriber("test/batching", channels::FifoChannel(3));
    std::this_thread::sleep_for(1s);

    auto batch_guard = session1.start_batching();

    session1.put("test/batching", "data1");
    session1.put("*/batching", "data2");
    std::this_thread::sleep_for(1s);

    auto res = subscriber.handler().try_recv();
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA);

    batch_guard.flush();
    std::this_thread::sleep_for(1s);

    assert(std::holds_alternative<Sample>(subscriber.handler().try_recv()));
    assert(std::holds_alternative<Sample>(subscriber.handler().try_recv()));
}

void test_batching_drop() {
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    auto subscriber = session2.declare_subscriber("test/batching", channels::FifoChannel(3));
    std::this_thread::sleep_for(1s);

    {
        auto batch_guard = session1.start_batching();

        session1.put("test/batching", "data1");
        session1.put("*/batching", "data2");
        std::this_thread::sleep_for(1s);

        auto res = subscriber.handler().try_recv();
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA);
    }

    std::this_thread::sleep_for(1s);

    assert(std::holds_alternative<Sample>(subscriber.handler().try_recv()));
    assert(std::holds_alternative<Sample>(subscriber.handler().try_recv()));
}

int main(int argc, char** argv) {
    test_multiple_batching();
    test_batching();
    test_batching_drop();
};
