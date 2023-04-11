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

int gcnt = 0;

struct OnReply {
    void operator()(Reply&&) {
        cnt++;
        gcnt++;
    }
    int cnt = 0;
};

struct OnReplyMove : public OnReply {
    OnReplyMove() {}
    OnReplyMove(const OnReplyMove&) = delete;
    OnReplyMove(OnReplyMove&& v) {
        cnt = v.cnt;
        v.cnt = 0;
    }
};

int main(int argc, char** argv) {
    int cnt = 0;

    OnReply on_reply;
    ClosureReply wrap_ref(on_reply);
    ClosureReply wrap_copy{OnReply()};

    OnReplyMove on_reply_move;
    ClosureReply wrap_move_ref(on_reply_move);
    ClosureReply wrap_move_rv{OnReplyMove()};

    Reply r(nullptr);
    wrap_ref(r);
    wrap_copy(r);
    wrap_move_ref(r);
    wrap_move_rv(r);

    assert(gcnt == 4);
    assert(on_reply.cnt == 1);  // Make sure that wrap_ref really wrapped on_reply by reference
    assert(on_reply_move.cnt == 1);

    ClosureReply wrap_move_lv(std::move(on_reply_move));
    wrap_move_lv(r);
    assert(gcnt == 5);
    assert(on_reply_move.cnt == 0);
}
