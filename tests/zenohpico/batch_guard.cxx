//
// Copyright (c) 2026 Yong Ling
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//

#include <utility>

#include "test_config.hxx"
#include "zenoh.hxx"

#undef NDEBUG
#include <assert.h>

using namespace zenoh;

#if Z_FEATURE_BATCHING == 1

void assert_invalid(const Session::BatchGuard& guard) {
    ZResult err = Z_OK;
    guard.flush(&err);
    assert(err == Z_EINVAL);
}

void test_repeated_batches() {
    auto [listener, session] = open_linked_session_pair();
    auto publisher = session.declare_publisher("test/batch_guard");
    for (int i = 0; i < 3; ++i) {
        {
            auto guard = session.start_batching();
            publisher.put("batched");
            if (i == 0) guard.flush();
        }
        assert(!session.is_closed());
        publisher.put("unbatched");
        auto another_publisher = session.declare_publisher("test/batch_guard/after_stop");
        another_publisher.put("after stop");
    }
}

void test_move_construction() {
    auto [listener, session] = open_linked_session_pair();
    {
        auto original = session.start_batching();
        {
            auto moved = std::move(original);
            assert_invalid(original);
            session.put("test/batch_guard", "moved");
            moved.flush();
        }
        assert(!session.is_closed());
        auto next = session.start_batching();
        next.flush();
    }
    assert(!session.is_closed());
    session.put("test/batch_guard", "after move");
}

void test_move_assignment() {
    auto [listener1, session1] = open_linked_session_pair();
    auto [listener2, session2] = open_linked_session_pair();
    {
        auto first = session1.start_batching();
        auto second = session2.start_batching();
        session1.put("test/batch_guard", "first");
        first = std::move(second);
        assert_invalid(second);
        assert(!session1.is_closed());
        // Assignment must stop the previous batch before taking ownership of the second one.
        auto next = session1.start_batching();
        next.flush();
        session2.put("test/batch_guard", "second");
        first.flush();
        auto& self = first;
        first = std::move(self);
        first.flush();
    }
    assert(!session1.is_closed());
    assert(!session2.is_closed());
    auto next = session2.start_batching();
    next.flush();
}

void test_failed_start() {
    auto [listener, session] = open_linked_session_pair();
    {
        auto guard = session.start_batching();
        {
            ZResult err = Z_OK;
            auto failed = session.start_batching(&err);
            assert(err != Z_OK);
            assert_invalid(failed);
        }
        ZResult err = Z_OK;
        auto still_active = session.start_batching(&err);
        assert(err != Z_OK);
        guard.flush();
    }
    assert(!session.is_closed());
    auto next = session.start_batching();
    next.flush();
}

int main() {
    test_repeated_batches();
    test_move_construction();
    test_move_assignment();
    test_failed_start();
}

#else

int main() {}

#endif
