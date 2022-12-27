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

#include "zenohcpp.h"

using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void get_options() {
    GetOptions opts;
    opts.set_consolidation(QueryConsolidation())
        .set_consolidation(Z_CONSOLIDATION_MODE_AUTO)
        .set_target(Z_QUERY_TARGET_ALL)
        .set_with_value("TEST");

    GetOptions opts2 = opts;
    assert(opts2 == opts);
    assert(opts.get_consolidation() == QueryConsolidation(Z_CONSOLIDATION_MODE_AUTO));
    assert(opts.get_target() == Z_QUERY_TARGET_ALL);
    assert(opts.get_with_value() == Value("TEST"));
}

int main(int argc, char** argv) { get_options(); };
