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
#include <unordered_set>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

#if defined Z_FEATURE_UNSTABLE_API

void test_liveliness_get() {
    KeyExpr ke("zenoh/liveliness/test/*");
    KeyExpr token_ke("zenoh/liveliness/test/1");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    auto token = session1.liveliness_declare_token(token_ke);
    std::this_thread::sleep_for(1s);

    auto replies = session2.liveliness_get(ke, channels::FifoChannel(3));
    auto res = replies.recv();
    assert(std::holds_alternative<Reply>(res));
    assert(std::get<Reply>(res).is_ok());
    assert(std::get<Reply>(res).get_ok().get_keyexpr() == "zenoh/liveliness/test/1");
    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);

    std::move(token).undeclare();
    std::this_thread::sleep_for(1s);

    replies = session2.liveliness_get(ke, channels::FifoChannel(3));
    res = replies.recv();
    assert(std::holds_alternative<channels::RecvError>(res));
    assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
}

void test_liveliness_subscriber() {
    KeyExpr ke("zenoh/liveliness/test/*");
    KeyExpr token_ke1("zenoh/liveliness/test/1");
    KeyExpr token_ke2("zenoh/liveliness/test/2");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::unordered_set<std::string> put_tokens;
    std::unordered_set<std::string> delete_tokens;

    auto subscriber = session1.liveliness_declare_subscriber(
        ke,
        [&put_tokens, &delete_tokens](const Sample& s) {
            if (s.get_kind() == Z_SAMPLE_KIND_PUT) {
                put_tokens.insert(std::string(s.get_keyexpr().as_string_view()));
            } else if (s.get_kind() == Z_SAMPLE_KIND_DELETE) {
                delete_tokens.insert(std::string(s.get_keyexpr().as_string_view()));
            }
        },
        closures::none);
    std::this_thread::sleep_for(1s);

    auto token1 = session2.liveliness_declare_token(token_ke1);
    auto token2 = session2.liveliness_declare_token(token_ke2);
    std::this_thread::sleep_for(1s);
    assert(put_tokens.size() == 2);
    assert(put_tokens.count("zenoh/liveliness/test/1") == 1);
    assert(put_tokens.count("zenoh/liveliness/test/2") == 1);

    std::move(token1).undeclare();
    std::this_thread::sleep_for(1s);

    assert(delete_tokens.size() == 1);
    assert(delete_tokens.count("zenoh/liveliness/test/1") == 1);

    std::move(token2).undeclare();
    std::this_thread::sleep_for(1s);
    assert(delete_tokens.size() == 2);
    assert(delete_tokens.count("zenoh/liveliness/test/2") == 1);
}

#endif

int main(int argc, char** argv) {
#if defined Z_FEATURE_UNSTABLE_API
    test_liveliness_get();
    test_liveliness_subscriber();
#endif
};
