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
#undef NDEBUG
#include <assert.h>

using namespace zenoh;
using namespace std::chrono_literals;

void pub_sub() {
    std::cout << "Test source info: pub_sub\n";
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke = "test/source_info/pub_sub";
    auto publisher = session1.declare_publisher(ke);
    auto subscriber = session2.declare_subscriber(ke, channels::FifoChannel(16));

    std::this_thread::sleep_for(1s);
    {
        publisher.put("data");
        std::this_thread::sleep_for(1s);

        auto s = subscriber.handler().try_recv();
        assert(std::holds_alternative<Sample>(s));
        auto source_info = std::get<Sample>(s).get_source_info();
        assert(!source_info.has_value());
    }

    {
        EntityGlobalId id = publisher.get_id();
        uint32_t sn = 15;

        Publisher::PutOptions opts;
        opts.source_info = SourceInfo(id, sn);

        publisher.put("data", std::move(opts));

        std::this_thread::sleep_for(1s);

        auto s = subscriber.handler().try_recv();
        assert(std::holds_alternative<Sample>(s));
        auto source_info = std::get<Sample>(s).get_source_info();
        assert(source_info.has_value());

        assert(source_info->get().sn() == sn);
        assert(source_info->get().id() == id);
    }
}

void put_sub() {
    std::cout << "Test source info: put_sub\n";
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke = "test/source_info/put_sub";
    auto subscriber = session2.declare_subscriber(ke, channels::FifoChannel(16));

    std::this_thread::sleep_for(1s);

    {
        session1.put(ke, "data");
        std::this_thread::sleep_for(1s);

        auto s = subscriber.handler().try_recv();
        assert(std::holds_alternative<Sample>(s));
        auto source_info = std::get<Sample>(s).get_source_info();
        assert(!source_info.has_value());
    }
    {
        EntityGlobalId id = session1.get_id();
        uint32_t sn = 15;

        Session::PutOptions opts;
        opts.source_info = SourceInfo(id, sn);

        session1.put(ke, "data", std::move(opts));

        std::this_thread::sleep_for(1s);

        auto s = subscriber.handler().try_recv();
        assert(std::holds_alternative<Sample>(s));
        auto source_info = std::get<Sample>(s).get_source_info();
        assert(source_info.has_value());

        assert(source_info->get().sn() == sn);
        assert(source_info->get().id() == id);
    }
}

void query_reply() {
    std::cout << "Test source info: query_reply\n";
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke = "test/source_info/query_reply";
    auto queryable = session2.declare_queryable(ke, channels::FifoChannel(16));

    std::this_thread::sleep_for(1s);
    {
        auto replies = session1.get(ke, "", channels::FifoChannel(16));
        std::this_thread::sleep_for(1s);
        {
            auto q = queryable.handler().try_recv();
            assert(std::holds_alternative<Query>(q));
            auto source_info = std::get<Query>(q).get_source_info();
            assert(!source_info.has_value());

            std::get<Query>(q).reply(ke, "ok");
        }

        std::this_thread::sleep_for(1s);
        auto r = replies.try_recv();
        assert(std::holds_alternative<Reply>(r));
        assert(std::get<Reply>(r).is_ok());
        auto source_info = std::get<Reply>(r).get_ok().get_source_info();
        assert(!source_info.has_value());
    }

    {
        EntityGlobalId id1 = session1.get_id();
        uint32_t sn1 = 15;

        Session::GetOptions opts1;
        opts1.source_info = SourceInfo(id1, sn1);

        EntityGlobalId id2 = queryable.get_id();
        uint32_t sn2 = 115;

        Query::ReplyOptions opts2;
        opts2.source_info = SourceInfo(id2, sn2);

        auto replies = session1.get(ke, "", channels::FifoChannel(16), std::move(opts1));
        std::this_thread::sleep_for(1s);
        {
            auto q = queryable.handler().try_recv();
            assert(std::holds_alternative<Query>(q));
            auto source_info = std::get<Query>(q).get_source_info();
            assert(source_info.has_value());
            assert(source_info->get().sn() == sn1);
            assert(source_info->get().id() == id1);
            std::get<Query>(q).reply(ke, "ok", std::move(opts2));
        }

        std::this_thread::sleep_for(1s);

        auto r = replies.try_recv();
        assert(std::holds_alternative<Reply>(r));
        assert(std::get<Reply>(r).is_ok());
        auto source_info = std::get<Reply>(r).get_ok().get_source_info();
        assert(source_info.has_value());
        assert(source_info->get().sn() == sn2);
        assert(source_info->get().id() == id2);
    }
}

void querier_reply() {
    std::cout << "Test source info: querier_reply\n";
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    KeyExpr ke = "test/source_info/querier_reply";
    auto querier = session1.declare_querier(ke);
    auto queryable = session2.declare_queryable(ke, channels::FifoChannel(16));

    std::this_thread::sleep_for(1s);
    {
        auto replies = querier.get("", channels::FifoChannel(16));
        std::this_thread::sleep_for(1s);
        {
            auto q = queryable.handler().try_recv();
            assert(std::holds_alternative<Query>(q));
            auto source_info = std::get<Query>(q).get_source_info();
            assert(!source_info.has_value());

            std::get<Query>(q).reply(ke, "ok");
        }

        std::this_thread::sleep_for(1s);
        auto r = replies.try_recv();
        assert(std::holds_alternative<Reply>(r));
        assert(std::get<Reply>(r).is_ok());
        auto source_info = std::get<Reply>(r).get_ok().get_source_info();
        assert(!source_info.has_value());
    }

    {
        EntityGlobalId id1 = querier.get_id();
        uint32_t sn1 = 15;

        Querier::GetOptions opts1;
        opts1.source_info = SourceInfo(id1, sn1);

        EntityGlobalId id2 = queryable.get_id();
        uint32_t sn2 = 115;

        Query::ReplyOptions opts2;
        opts2.source_info = SourceInfo(id2, sn2);

        auto replies = querier.get("", channels::FifoChannel(16), std::move(opts1));
        std::this_thread::sleep_for(1s);
        {
            auto q = queryable.handler().try_recv();
            assert(std::holds_alternative<Query>(q));
            auto source_info = std::get<Query>(q).get_source_info();
            assert(source_info.has_value());
            assert(source_info->get().sn() == sn1);
            assert(source_info->get().id() == id1);
            std::get<Query>(q).reply(ke, "ok", std::move(opts2));
        }

        std::this_thread::sleep_for(1s);

        auto r = replies.try_recv();
        assert(std::holds_alternative<Reply>(r));
        assert(std::get<Reply>(r).is_ok());
        auto source_info = std::get<Reply>(r).get_ok().get_source_info();
        assert(source_info.has_value());
        assert(source_info->get().sn() == sn2);
        assert(source_info->get().id() == id2);
    }
}

int main(int argc, char** argv) {
    pub_sub();
    put_sub();
    query_reply();
    querier_reply();
}
