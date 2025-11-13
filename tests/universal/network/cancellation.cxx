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

void test_get() {
    std::cout << "test_get\n";

    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke("zenoh-cpp/query/cancellation/test");
    auto queryable = session1.declare_queryable(ke, channels::FifoChannel(3));
    std::this_thread::sleep_for(1s);

    std::cout << "Check cancel removes callbacks\n";
    {
        CancellationToken ct;
        assert(!ct.is_cancelled());

        Session::GetOptions opt;
        opt.cancellation_token = ct;
        auto replies = session2.get(ke, "", channels::FifoChannel(3), std::move(opt));
        std::this_thread::sleep_for(1s);
        ct.cancel();
        assert(ct.is_cancelled());
        auto res = replies.try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
        queryable.handler().try_recv();
    }

    std::cout << "Check cancel blocks until callback is finished\n";
    {
        CancellationToken ct;

        Session::GetOptions opt;
        opt.cancellation_token = ct;
        size_t val = 0;
        session2.get(
            ke, "",
            [&val](const Reply&) {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            [&val]() {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            std::move(opt));
        std::this_thread::sleep_for(1s);
        {
            auto q = std::get<Query>(queryable.handler().try_recv());
            q.reply(ke, "ok");
        }
        std::this_thread::sleep_for(1s);
        ct.cancel();
        assert(val == 2);
    }

    std::cout << "Check cancelled token does not send a query\n";
    {
        CancellationToken ct;
        assert(!ct.is_cancelled());
        ct.cancel();
        assert(ct.is_cancelled());

        Session::GetOptions opt;
        opt.cancellation_token = ct;
        auto replies = session2.get(ke, "", channels::FifoChannel(3), std::move(opt));
        std::this_thread::sleep_for(1s);
        auto res = replies.try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
    }
}

void test_querier_get() {
    std::cout << "test_querier_get\n";

    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke("zenoh-cpp/querier/cancellation/test");
    auto queryable = session1.declare_queryable(ke, channels::FifoChannel(3));
    auto querier = session2.declare_querier(ke);
    std::this_thread::sleep_for(1s);

    std::cout << "Check cancel removes callbacks\n";
    {
        CancellationToken ct;
        assert(!ct.is_cancelled());

        Querier::GetOptions opt;
        opt.cancellation_token = ct;
        auto replies = querier.get("", channels::FifoChannel(3), std::move(opt));
        std::this_thread::sleep_for(1s);
        ct.cancel();
        assert(ct.is_cancelled());
        auto res = replies.try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
        queryable.handler().try_recv();
    }

    std::cout << "Check cancel blocks until callback is finished\n";
    {
        CancellationToken ct;

        Querier::GetOptions opt;
        opt.cancellation_token = ct;
        size_t val = 0;
        querier.get(
            "",
            [&val](const Reply&) {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            [&val]() {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            std::move(opt));
        std::this_thread::sleep_for(1s);
        {
            auto q = std::get<Query>(queryable.handler().try_recv());
            q.reply(ke, "ok");
        }
        std::this_thread::sleep_for(1s);
        ct.cancel();
        assert(val == 2);
    }

    std::cout << "Check cancelled token does not send a query\n";
    {
        CancellationToken ct;
        assert(!ct.is_cancelled());
        ct.cancel();
        assert(ct.is_cancelled());

        Querier::GetOptions opt;
        opt.cancellation_token = ct;
        auto replies = querier.get("", channels::FifoChannel(3), std::move(opt));
        std::this_thread::sleep_for(1s);
        auto res = replies.try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
    }
}

void test_liveliness_get() {
    std::cout << "test_liveliness_get\n";

    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke("zenoh-cpp/liveliness/cancellation/test");
    auto liveliness_token = session1.liveliness_declare_token(ke);
    std::this_thread::sleep_for(1s);

    std::cout << "Check cancel blocks until callback is finished\n";
    {
        CancellationToken ct;

        Session::LivelinessGetOptions opt;
        opt.cancellation_token = ct;
        size_t val = 0;
        session2.liveliness_get(
            ke,
            [&val](const Reply&) {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            [&val]() {
                std::this_thread::sleep_for(1s);
                val += 1;
                std::this_thread::sleep_for(1s);
            },
            std::move(opt));
        std::this_thread::sleep_for(1s);
        ct.cancel();
        assert(val == 2);
    }

    std::cout << "Check cancelled token does not send a query\n";
    {
        CancellationToken ct;
        assert(!ct.is_cancelled());
        ct.cancel();
        assert(ct.is_cancelled());

        Session::LivelinessGetOptions opt;
        opt.cancellation_token = ct;
        auto replies = session2.liveliness_get(ke, channels::FifoChannel(3), std::move(opt));
        std::this_thread::sleep_for(1s);
        auto res = replies.try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
    }
}

int main(int argc, char** argv) {
    test_get();
    test_querier_get();
    test_liveliness_get();
};
