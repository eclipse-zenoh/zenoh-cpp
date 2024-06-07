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
#include <thread>

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

struct QueryData {
    std::string key;
    std::string params;
    int32_t payload;
    bool operator==(const QueryData &other) {
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
    auto session1 = Session::open(Config());
    auto session2 = Session::open(Config());
    size_t queries_processed = 0;

    {
        auto queryable = session1.declare_queryable(
            ke,
            [&queries](const Query& q) {
                auto val = q.get_value().get_payload().deserialize<int32_t>();
                queries.push_back(QueryData{.key = std::string(q.get_keyexpr().as_string_view()), .params = std::string(q.get_parameters()), .payload = val});
                if (q.get_parameters() == "ok") {
                    q.reply(q.get_keyexpr(), Bytes::serialize(std::to_string(val)));
                } else {
                    q.reply_err(Bytes::serialize("err"));
                }
            },
            [&queryable_dropped]() { queryable_dropped = true; }
        );
        std::this_thread::sleep_for(1s);

        auto on_reply = [&replies, &errors](const Reply& r) {
            if (r.is_ok()) {
                replies.push_back(r.get_ok().get_payload().deserialize<std::string>());
            } else {
                errors.push_back(r.get_err().get_payload().deserialize<std::string>());
            }
        };
        auto on_drop = [&queries_processed]() { queries_processed++; };
    
        session2.get(selector, "ok", on_reply, on_drop, {.payload = Bytes::serialize<int32_t>(1) });
        std::this_thread::sleep_for(1s);
        session2.get(selector, "ok", on_reply, on_drop, {.payload = Bytes::serialize<int32_t>(2) });
        std::this_thread::sleep_for(1s);
        session2.get(selector, "err", on_reply, on_drop, {.payload = Bytes::serialize<int32_t>(3) });
        std::this_thread::sleep_for(1s);
    }

    assert(queries.size() == 3);
    QueryData qd = { "zenoh/test/1", "ok", 1 };
    assert(queries[0] == qd);
    qd = { "zenoh/test/1", "ok", 2 };
    assert(queries[1] == qd);
    qd = { "zenoh/test/1", "err", 3 };
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
    auto session1 = Session::open(Config());
    auto session2 = Session::open(Config());
    size_t queries_processed = 0;
    auto queryable = session1.declare_queryable(
        ke,
        [&queries](const Query& q) {
            auto val = q.get_value().get_payload().deserialize<int32_t>();
            queries.push_back(QueryData{.key = std::string(q.get_keyexpr().as_string_view()), .params = std::string(q.get_parameters()), .payload = val});
            if (q.get_parameters() == "ok") {
                q.reply(q.get_keyexpr(), Bytes::serialize(std::to_string(val)));
            } else {
                q.reply_err(Bytes::serialize("err"));
            }
        },
        closures::none
    );
    std::this_thread::sleep_for(1s);

    auto replies = session2.get(selector, "ok", channels::FifoChannel(3), {.payload = Bytes::serialize<int32_t>(1) });
    auto reply = replies.recv().first;
    assert(static_cast<bool>(reply));
    assert(reply.is_ok());
    assert(reply.get_ok().get_payload().deserialize<std::string>() == "1");
    assert(reply.get_ok().get_keyexpr().as_string_view() == "zenoh/test/1");
    reply = replies.recv().first;
    assert(!replies.recv().first);

    replies = session2.get(selector, "err", channels::FifoChannel(3), {.payload = Bytes::serialize<int32_t>(3) });
    reply = replies.recv().first;
    assert(static_cast<bool>(reply));
    assert(!reply.is_ok());
    assert(reply.get_err().get_payload().deserialize<std::string>() == "err");
    assert(!replies.recv().first);
}



int main(int argc, char** argv) {
    queryable_get();       
    queryable_get_channel();
}
