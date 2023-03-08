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

    std::vector<const char*> sv2 = {"foo", "bar", "buzz"};
    StrArrayView v2(sv2);

    const char* sv3[] = {"foo", "bar", "buzz"};
    StrArrayView v3(sv3, 3);

    char foo[] = "foo";
    char bar[] = "bar";
    char buzz[] = "buzz";
    char* sv4[] = {foo, bar, buzz};
    StrArrayView v4(sv4, 3);
}

int main(int argc, char** argv) { str_array_view(); };
