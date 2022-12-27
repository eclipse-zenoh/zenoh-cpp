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

struct OnQuery {
    void operator()(const Query*) {
        cnt++;
        gcnt++;
    }
    int cnt = 0;
};

struct OnQueryMove : public OnQuery {
    OnQueryMove() {}
    OnQueryMove(const OnQueryMove&) = delete;
    OnQueryMove(OnQueryMove&& v) {
        cnt = v.cnt;
        v.cnt = 0;
    }
};

int main(int argc, char** argv) {
    int cnt = 0;

    OnQuery on_query;
    ClosureQuery wrap_ref(on_query);
    ClosureQuery wrap_copy{OnQuery()};

    OnQueryMove on_query_move;
    ClosureQuery wrap_move_ref(on_query_move);
    ClosureQuery wrap_move_rv{OnQueryMove()};

    wrap_ref(nullptr);
    wrap_copy(nullptr);
    wrap_move_ref(nullptr);
    wrap_move_rv(nullptr);

    assert(gcnt == 4);
    assert(on_query.cnt == 1);  // Make sure that wrap_ref really wrapped on_query by reference
    assert(on_query_move.cnt == 1);

    ClosureQuery wrap_move_lv(std::move(on_query_move));
    wrap_move_lv(nullptr);
    assert(gcnt == 5);
    assert(on_query_move.cnt == 0);
}
