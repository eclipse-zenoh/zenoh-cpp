//
// Copyright (c) 2023 ZettaScale Technology
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

#include "zenohc.hxx"

using namespace zenohc;

#undef NDEBUG
#include <assert.h>

void test_session_rcinc() {
    Config config;
    auto s1 = expect<Session>(open(std::move(config)));
    auto s2 = s1.rcinc();
    assert(s1.check());
    assert(s2.check());
    s1 = Session(nullptr);
    assert(!s1.check());
    assert(s2.check());
    s2 = Session(nullptr);
    assert(!s1.check());
    assert(!s2.check());

    s1 = s2.rcinc();
    assert(!s1.check());
    assert(!s2.check());
}

int main(int argc, char** argv) {
    init_logger();
    test_session_rcinc();
};
