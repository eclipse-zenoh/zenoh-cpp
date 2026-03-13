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

#include <chrono>
#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

struct QueryData {
    std::string key;
    std::string params;
    std::string payload;
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

    auto queryable = session1.declare_queryable(
        ke,
        [&queries](const Query& q) {
            auto payload = q.get_payload()->get().as_string();
            QueryData qd;
            qd.key = std::string(q.get_keyexpr().as_string_view());
            qd.params = std::string(q.get_parameters());
            qd.payload = payload;
            queries.push_back(std::move(qd));
            if (q.get_parameters() == "ok") {
                q.reply(q.get_keyexpr(), Bytes(payload));
            } else {
                q.reply_err(Bytes("err"));
            }
        },
        [&queryable_dropped]() { queryable_dropped = true; });
    std::this_thread::sleep_for(1s);

    auto on_reply = [&replies, &errors](const Reply& r) {
        if (r.is_ok()) {
            replies.push_back(r.get_ok().get_payload().as_string());
        } else {
            errors.push_back(r.get_err().get_payload().as_string());
        }
    };
    auto on_drop = [&queries_processed]() { queries_processed++; };

    Session::GetOptions opt1;
    opt1.payload = Bytes("1");
    session2.get(selector, "ok", on_reply, on_drop, std::move(opt1));
    std::this_thread::sleep_for(1s);

    Session::GetOptions opt2;
    opt2.payload = Bytes("2");
    session2.get(selector, "ok", on_reply, on_drop, std::move(opt2));
    std::this_thread::sleep_for(1s);

    Session::GetOptions opt3;
    opt3.payload = Bytes("3");
    session2.get(selector, "err", on_reply, on_drop, std::move(opt3));
    std::this_thread::sleep_for(1s);

    std::move(queryable).undeclare();

    assert(queries.size() == 3);
    QueryData qd = {"zenoh/test/1", "ok", "1"};
    assert(queries[0] == qd);
    qd = {"zenoh/test/1", "ok", "2"};
    assert(queries[1] == qd);
    qd = {"zenoh/test/1", "err", "3"};
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
    opt1.payload = "1";
    auto replies = session2.get(selector, "ok", channels::FifoChannel(3), std::move(opt1));
    {
        auto res = queryable.handler().recv();
        assert(std::holds_alternative<Query>(res));
        auto& query = std::get<Query>(res);
        assert(query.get_keyexpr() == selector);
        assert(query.get_parameters() == "ok");
        assert(query.get_payload()->get().as_string() == "1");
        query.reply(query.get_keyexpr(), Bytes("1"));
    }

    auto res = replies.recv();
    assert(std::holds_alternative<Reply>(res));
    assert(std::get<Reply>(res).is_ok());
    assert(std::get<Reply>(res).get_ok().get_payload().as_string() == "1");
    assert(std::get<Reply>(res).get_ok().get_keyexpr().as_string_view() == "zenoh/test/1");

    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);

    Session::GetOptions opt3;
    opt3.payload = Bytes("3");
    replies = session2.get(selector, "err", channels::FifoChannel(3), std::move(opt3));
    {
        auto res = queryable.handler().recv();
        assert(std::holds_alternative<Query>(res));
        auto& query = std::get<Query>(res);
        assert(query.get_keyexpr() == selector);
        assert(query.get_parameters() == "err");
        assert(query.get_payload()->get().as_string() == "3");
        query.reply_err(Bytes("err"));
    }

    res = replies.recv();
    assert(std::holds_alternative<Reply>(res));
    assert(!std::get<Reply>(res).is_ok());
    assert(std::get<Reply>(res).get_err().get_payload().as_string() == "err");
    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
    std::move(queryable).undeclare();
}

void queryable_get_keyexpr() {
    KeyExpr ke("zenoh/test_queryable_keyexpr");
    auto session = Session::open(Config::create_default());
    auto queryable = session.declare_queryable(ke, channels::FifoChannel(3));
    assert(queryable.get_keyexpr().as_string_view() == "zenoh/test_queryable_keyexpr");
}

// Test that accept_replies field of Session::GetOptions works correctly.
//
// A queryable is declared on "zenoh/test/accept_replies/*".
// Queries are sent for "zenoh/test/accept_replies/1", but the queryable replies with
// the disjoint key expression "zenoh/test/accept_replies/2".
//
// With the default accept_replies (Z_REPLY_KEYEXPR_MATCHING_QUERY), the reply attempt
// should fail and no reply should be received by the getter.
//
// With accept_replies set to Z_REPLY_KEYEXPR_ANY, the disjoint reply should succeed
// and be received by the getter.
void queryable_get_accept_replies() {
    KeyExpr ke("zenoh/test/accept_replies/*");
    KeyExpr query_ke("zenoh/test/accept_replies/1");
    KeyExpr disjoint_reply_ke("zenoh/test/accept_replies/2");

    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    // --- Test 1: default accept_replies (Z_REPLY_KEYEXPR_MATCHING_QUERY) ---
    // The queryable inspects the query's accept_replies setting and verifies it matches
    // the expected value. It then attempts to reply with a disjoint key expression,
    // which should be rejected.
    {
        ReplyKeyExpr observed_accept_replies = Z_REPLY_KEYEXPR_ANY;
        ZResult reply_result = 0;

        auto queryable = session1.declare_queryable(
            ke,
            [&](const Query& q) {
                observed_accept_replies = q.get_accepts_replies();
                // Attempt to reply with a disjoint key expression; this should fail
                // when accept_replies is Z_REPLY_KEYEXPR_MATCHING_QUERY.
                q.reply(disjoint_reply_ke, Bytes("disjoint"), Query::ReplyOptions::create_default(), &reply_result);
            },
            []() {});
        std::this_thread::sleep_for(1s);

        std::vector<std::string> replies;
        Session::GetOptions opts;
        // leave opts.accept_replies at its default (Z_REPLY_KEYEXPR_MATCHING_QUERY)
        session2.get(
            query_ke, "",
            [&replies](const Reply& r) {
                if (r.is_ok()) replies.push_back(r.get_ok().get_payload().as_string());
            },
            []() {}, std::move(opts));
        std::this_thread::sleep_for(1s);

        std::move(queryable).undeclare();

        // The query should have been received with the default accept_replies value.
        assert(observed_accept_replies == Z_REPLY_KEYEXPR_MATCHING_QUERY);
        // Replying with a disjoint key expression should have failed.
        assert(reply_result != Z_OK);
        // No replies should have been received.
        assert(replies.empty());
    }

    // --- Test 2: accept_replies = Z_REPLY_KEYEXPR_ANY ---
    // The queryable replies with a disjoint key expression, which should now be accepted.
    {
        ReplyKeyExpr observed_accept_replies = Z_REPLY_KEYEXPR_MATCHING_QUERY;
        ZResult reply_result = -1;

        auto queryable = session1.declare_queryable(
            ke,
            [&](const Query& q) {
                observed_accept_replies = q.get_accepts_replies();
                // Attempt to reply with a disjoint key expression; this should succeed
                // when accept_replies is Z_REPLY_KEYEXPR_ANY.
                q.reply(disjoint_reply_ke, Bytes("disjoint"), Query::ReplyOptions::create_default(), &reply_result);
            },
            []() {});
        std::this_thread::sleep_for(1s);

        std::vector<std::string> replies;
        std::vector<std::string> reply_keyexprs;
        Session::GetOptions opts;
        opts.accept_replies = Z_REPLY_KEYEXPR_ANY;
        session2.get(
            query_ke, "",
            [&](const Reply& r) {
                if (r.is_ok()) {
                    replies.push_back(r.get_ok().get_payload().as_string());
                    reply_keyexprs.push_back(std::string(r.get_ok().get_keyexpr().as_string_view()));
                }
            },
            []() {}, std::move(opts));
        std::this_thread::sleep_for(1s);

        std::move(queryable).undeclare();

        // The query should have been received with the Z_REPLY_KEYEXPR_ANY setting.
        assert(observed_accept_replies == Z_REPLY_KEYEXPR_ANY);
        // Replying with a disjoint key expression should have succeeded.
        assert(reply_result == Z_OK);
        // The disjoint reply should have been received.
        assert(replies.size() == 1);
        assert(replies[0] == "disjoint");
        assert(reply_keyexprs[0] == "zenoh/test/accept_replies/2");
    }
}

// Test that accept_replies field of Session::QuerierOptions works correctly.
//
// This is similar to queryable_get_accept_replies(), but uses a Querier instead of
// Session::get(). With a Querier the accept_replies policy is fixed at declaration time
// via Session::QuerierOptions and applies to every subsequent Querier::get() call.
//
// A queryable is declared on "zenoh/test/querier_accept_replies/*".
// Queries are issued for "zenoh/test/querier_accept_replies/1", but the queryable replies
// with the disjoint key expression "zenoh/test/querier_accept_replies/2".
//
// With the default accept_replies (Z_REPLY_KEYEXPR_MATCHING_QUERY), the reply attempt
// should fail and no reply should be received by the querier.
//
// With accept_replies set to Z_REPLY_KEYEXPR_ANY in QuerierOptions, the disjoint reply
// should succeed and be received.
void queryable_querier_accept_replies() {
    KeyExpr ke("zenoh/test/querier_accept_replies/*");
    KeyExpr query_ke("zenoh/test/querier_accept_replies/1");
    KeyExpr disjoint_reply_ke("zenoh/test/querier_accept_replies/2");

    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    // --- Test 1: default accept_replies (Z_REPLY_KEYEXPR_MATCHING_QUERY) ---
    {
        ReplyKeyExpr observed_accept_replies = Z_REPLY_KEYEXPR_ANY;
        ZResult reply_result = 0;

        auto queryable = session1.declare_queryable(
            ke,
            [&](const Query& q) {
                observed_accept_replies = q.get_accepts_replies();
                // Attempt to reply with a disjoint key expression; this should fail
                // when accept_replies is Z_REPLY_KEYEXPR_MATCHING_QUERY.
                q.reply(disjoint_reply_ke, Bytes("disjoint"), Query::ReplyOptions::create_default(), &reply_result);
            },
            []() {});
        std::this_thread::sleep_for(1s);

        // Declare querier with default options (accept_replies = Z_REPLY_KEYEXPR_MATCHING_QUERY).
        auto querier = session2.declare_querier(query_ke);
        std::this_thread::sleep_for(1s);

        std::vector<std::string> replies;
        querier.get(
            "",
            [&replies](const Reply& r) {
                if (r.is_ok()) replies.push_back(r.get_ok().get_payload().as_string());
            },
            []() {});
        std::this_thread::sleep_for(1s);

        std::move(queryable).undeclare();

        // The query should have been received with the default accept_replies value.
        assert(observed_accept_replies == Z_REPLY_KEYEXPR_MATCHING_QUERY);
        // Replying with a disjoint key expression should have failed.
        assert(reply_result != Z_OK);
        // No replies should have been received.
        assert(replies.empty());
    }

    // --- Test 2: accept_replies = Z_REPLY_KEYEXPR_ANY set via QuerierOptions ---
    {
        ReplyKeyExpr observed_accept_replies = Z_REPLY_KEYEXPR_MATCHING_QUERY;
        ZResult reply_result = -1;

        auto queryable = session1.declare_queryable(
            ke,
            [&](const Query& q) {
                observed_accept_replies = q.get_accepts_replies();
                // Attempt to reply with a disjoint key expression; this should succeed
                // when accept_replies is Z_REPLY_KEYEXPR_ANY.
                q.reply(disjoint_reply_ke, Bytes("disjoint"), Query::ReplyOptions::create_default(), &reply_result);
            },
            []() {});
        std::this_thread::sleep_for(1s);

        // Declare querier with accept_replies = Z_REPLY_KEYEXPR_ANY.
        Session::QuerierOptions querier_opts;
        querier_opts.accept_replies = Z_REPLY_KEYEXPR_ANY;
        auto querier = session2.declare_querier(query_ke, std::move(querier_opts));
        std::this_thread::sleep_for(1s);

        std::vector<std::string> replies;
        std::vector<std::string> reply_keyexprs;
        querier.get(
            "",
            [&](const Reply& r) {
                if (r.is_ok()) {
                    replies.push_back(r.get_ok().get_payload().as_string());
                    reply_keyexprs.push_back(std::string(r.get_ok().get_keyexpr().as_string_view()));
                }
            },
            []() {});
        std::this_thread::sleep_for(1s);

        std::move(queryable).undeclare();

        // The query should have been received with the Z_REPLY_KEYEXPR_ANY setting.
        assert(observed_accept_replies == Z_REPLY_KEYEXPR_ANY);
        // Replying with a disjoint key expression should have succeeded.
        assert(reply_result == Z_OK);
        // The disjoint reply should have been received.
        assert(replies.size() == 1);
        assert(replies[0] == "disjoint");
        assert(reply_keyexprs[0] == "zenoh/test/querier_accept_replies/2");
    }
}

int main(int argc, char** argv) {
    queryable_get();
    queryable_get_channel();
    queryable_get_keyexpr();
    queryable_get_accept_replies();
    queryable_querier_accept_replies();
}
