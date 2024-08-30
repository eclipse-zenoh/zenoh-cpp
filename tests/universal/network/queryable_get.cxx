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

#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

struct QueryData {
    std::string key;
    std::string params;
    int32_t payload;
    bool operator==(const QueryData& other) {
        return key == other.key && params == other.params && payload == other.payload;
    }
};

void queryable_get() {
    KeyExpr ke("zenoh/test/*");
    KeyExpr selector("zenoh/test/1");
    std::vector<QueryData> queries;
    std::vector<std::string> replies;
    std::vector<std::string> errors;
    bool queryable_dropped = false;
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    size_t queries_processed = 0;

    {
        auto queryable = session1.declare_queryable(
            ke,
            [&queries](const Query& q) {
                auto payload = q.get_payload()->get().deserialize<int32_t>();
                QueryData qd;
                qd.key = std::string(q.get_keyexpr().as_string_view());
                qd.params = std::string(q.get_parameters());
                qd.payload = payload;
                queries.push_back(std::move(qd));
                if (q.get_parameters() == "ok") {
                    q.reply(q.get_keyexpr(), Bytes::serialize(std::to_string(payload)));
                } else {
                    q.reply_err(Bytes::serialize("err"));
                }
            },
            [&queryable_dropped]() { queryable_dropped = true; });
        std::this_thread::sleep_for(1s);

        auto on_reply = [&replies, &errors](const Reply& r) {
            if (r.is_ok()) {
                replies.push_back(r.get_ok().get_payload().deserialize<std::string>());
            } else {
                errors.push_back(r.get_err().get_payload().deserialize<std::string>());
            }
        };
        auto on_drop = [&queries_processed]() { queries_processed++; };

        Session::GetOptions opt1;
        opt1.payload = Bytes::serialize<int32_t>(1);
        session2.get(selector, "ok", on_reply, on_drop, std::move(opt1));
        std::this_thread::sleep_for(1s);

        Session::GetOptions opt2;
        opt2.payload = Bytes::serialize<int32_t>(2);
        session2.get(selector, "ok", on_reply, on_drop, std::move(opt2));
        std::this_thread::sleep_for(1s);

        Session::GetOptions opt3;
        opt3.payload = Bytes::serialize<int32_t>(3);
        session2.get(selector, "err", on_reply, on_drop, std::move(opt3));
        std::this_thread::sleep_for(1s);
    }

    assert(queries.size() == 3);
    QueryData qd = {"zenoh/test/1", "ok", 1};
    assert(queries[0] == qd);
    qd = {"zenoh/test/1", "ok", 2};
    assert(queries[1] == qd);
    qd = {"zenoh/test/1", "err", 3};
    assert(queries[2] == qd);

    assert(queryable_dropped);

    assert(replies.size() == 2);
    assert(replies[0] == "1");
    assert(replies[1] == "2");

    assert(errors.size() == 1);
    assert(errors[0] == "err");

    assert(queries_processed == 3);
}

void queryable_get_channel() {
    KeyExpr ke("zenoh/test/*");
    KeyExpr selector("zenoh/test/1");
    std::vector<QueryData> queries;
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    size_t queries_processed = 0;
    auto queryable = session1.declare_queryable(ke, channels::FifoChannel(3));
    std::this_thread::sleep_for(1s);

    Session::GetOptions opt1;
    opt1.payload = Bytes::serialize<int32_t>(1);
    auto replies = session2.get(selector, "ok", channels::FifoChannel(3), std::move(opt1));
    {
        auto res = queryable.handler().recv();
        assert(std::holds_alternative<Query>(res));
        auto& query = std::get<Query>(res);
        assert(query.get_keyexpr() == selector);
        assert(query.get_parameters() == "ok");
        assert(query.get_payload()->get().deserialize<int32_t>() == 1);
        query.reply(query.get_keyexpr(), Bytes::serialize(std::to_string(1)));
    }

    auto res = replies.recv();
    assert(std::holds_alternative<Reply>(res));
    assert(std::get<Reply>(res).is_ok());
    assert(std::get<Reply>(res).get_ok().get_payload().deserialize<std::string>() == "1");
    assert(std::get<Reply>(res).get_ok().get_keyexpr().as_string_view() == "zenoh/test/1");

    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);

    Session::GetOptions opt3;
    opt3.payload = Bytes::serialize<int32_t>(3);
    replies = session2.get(selector, "err", channels::FifoChannel(3), std::move(opt3));
    {
        auto res = queryable.handler().recv();
        assert(std::holds_alternative<Query>(res));
        auto& query = std::get<Query>(res);
        assert(query.get_keyexpr() == selector);
        assert(query.get_parameters() == "err");
        assert(query.get_payload()->get().deserialize<int32_t>() == 3);
        query.reply_err(Bytes::serialize("err"));
    }

    res = replies.recv();
    assert(std::holds_alternative<Reply>(res));
    assert(!std::get<Reply>(res).is_ok());
    assert(std::get<Reply>(res).get_err().get_payload().deserialize<std::string>() == "err");
    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
}

int main(int argc, char** argv) {
    queryable_get();
    queryable_get_channel();
}
