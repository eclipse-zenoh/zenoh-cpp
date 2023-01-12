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
#include <assert.h>

#include "zenohcpp.h"

using namespace zenoh;

#undef NDEBUG
#include <assert.h>

int gcnt = 0;

void on_reply_lv(Reply) { gcnt++; };
void on_reply_rv(Reply&) { gcnt++; };
void on_reply_rlv(Reply&&) { gcnt++; };

void on_query_ptr(const Query*) { gcnt++; };

void on_sample_ptr(const Sample*) { gcnt++; };

void on_id_ptr(const Id*) { gcnt++; };

void on_hello_lv(Hello) { gcnt++; };
void on_hello_rv(Hello&) { gcnt++; };
void on_hello_rlv(Hello&&) { gcnt++; };

int main(int argc, char** argv) {
    int cnt = 0;

    ClosureReply closure_reply_lv([&cnt](Reply) { cnt++; });
    ClosureReply closure_reply_rv([&cnt](Reply&) { cnt++; });
    ClosureReply closure_reply_rlv([&cnt](Reply&&) { cnt++; });
    ClosureReply closure_reply_flv(on_reply_lv);
    ClosureReply closure_reply_frv(on_reply_rv);
    ClosureReply closure_reply_frlv(on_reply_rlv);

    Reply reply(nullptr);
    closure_reply_lv(reply);
    closure_reply_rv(reply);
    closure_reply_rlv(reply);
    closure_reply_flv(reply);
    closure_reply_frv(reply);
    closure_reply_frlv(reply);
    closure_reply_lv(std::move(reply));
    closure_reply_rv(std::move(reply));
    closure_reply_rlv(std::move(reply));
    closure_reply_flv(std::move(reply));
    closure_reply_frv(std::move(reply));
    closure_reply_frlv(std::move(reply));

    assert(cnt == 6);
    assert(gcnt == 6);
    cnt = gcnt = 0;

    ClosureQuery closure_query([&cnt](const Query*) { cnt++; });
    ClosureQuery closure_query_f(on_query_ptr);

    closure_query(nullptr);
    closure_query_f(nullptr);

    assert(cnt == 1);
    assert(gcnt == 1);
    cnt = gcnt = 0;

    ClosureSample closure_sample([&cnt](const Sample*) { cnt++; });
    ClosureSample closure_sample_f(on_sample_ptr);

    closure_sample(nullptr);
    closure_sample_f(nullptr);

    assert(cnt == 1);
    assert(gcnt == 1);
    cnt = gcnt = 0;

    ClosureZid closure_zid([&cnt](const Id*) { cnt++; });
    ClosureZid closure_zid_f(on_id_ptr);

    closure_zid(nullptr);
    closure_zid_f(nullptr);

    assert(cnt == 1);
    assert(gcnt == 1);
    cnt = gcnt = 0;

    ClosureHello closure_hello_lv([&cnt](Hello) { cnt++; });
    ClosureHello closure_hello_rv([&cnt](Hello&) { cnt++; });
    ClosureHello closure_hello_rlv([&cnt](Hello&&) { cnt++; });
    ClosureHello closure_hello_flv(on_hello_lv);
    ClosureHello closure_hello_frv(on_hello_rv);
    ClosureHello closure_hello_frlv(on_hello_rlv);

    Hello hello(nullptr);
    closure_hello_lv(hello);
    closure_hello_rv(hello);
    closure_hello_rlv(hello);
    closure_hello_flv(hello);
    closure_hello_frv(hello);
    closure_hello_frlv(hello);
    closure_hello_lv(std::move(hello));
    closure_hello_rv(std::move(hello));
    closure_hello_rlv(std::move(hello));
    closure_hello_flv(std::move(hello));
    closure_hello_frv(std::move(hello));
    closure_hello_frlv(std::move(hello));

    assert(cnt == 6);
    assert(gcnt == 6);
    cnt = gcnt = 0;
}
