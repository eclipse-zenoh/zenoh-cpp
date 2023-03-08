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

#include <vector>

#include "zenohcpp.h"

using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void str_array_view() {
    StrArrayView sv1;
    assert(sv1.get_len() == 0);

    std::vector<const char*> sv2 = {"foo", "bar", "buzz"};
    StrArrayView v2(sv2);
    assert(v2.get_len() == 3);
    assert(std::string("foo") == v2[0]);
    assert(std::string("bar") == v2[1]);
    assert(std::string("buzz") == v2[2]);

    const char* sv3[] = {"foo", "bar", "buzz"};
    StrArrayView v3(sv3, 3);
    assert(v3.get_len() == 3);
    assert(std::string("foo") == v3[0]);
    assert(std::string("bar") == v3[1]);
    assert(std::string("buzz") == v3[2]);

    char foo[] = "foo";
    char bar[] = "bar";
    char buzz[] = "buzz";
    char* sv4[] = {foo, bar, buzz};
    StrArrayView v4(sv4, 3);
    assert(v4.get_len() == 3);
    assert(std::string("foo") == v4[0]);
    assert(std::string("bar") == v4[1]);
    assert(std::string("buzz") == v4[2]);
}

void bytes_view() {}

int main(int argc, char** argv) { str_array_view(); };
