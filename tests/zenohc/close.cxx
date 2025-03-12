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

void test_session_close_in_drop() {
    auto session = Session::open(Config::create_default());
}

void test_session_close() {
    auto session = Session::open(Config::create_default());
    session.close();
}

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
void test_session_close_in_background() {
    auto session = Session::open(Config::create_default());

    bool close_called = false;
    auto close_options = Session::SessionCloseOptions::create_default();
    close_options.out_concurrent = [&close_called](CloseHandle&& h) { h.wait(); close_called = true; };

    session.close(std::move(close_options));

    if(!close_called) {
        exit(-1);
    }
}
#endif

int main() {
    test_session_close_in_drop();
    test_session_close();
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    test_session_close_in_background();
#endif
}