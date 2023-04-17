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

#include "zenohpico.hxx"

using namespace zenohpico;

#undef NDEBUG
#include <assert.h>

void test_config_default() {
    Config config;
    assert(config.get(Z_CONFIG_MULTICAST_LOCATOR_KEY) != nullptr);
}

void test_config_insert() {
    Config config;
    assert(config.insert(Z_CONFIG_USER_KEY, "foo"));
    assert(std::string("foo") == config.get(Z_CONFIG_USER_KEY));
}

int main(int argc, char** argv) {
    test_config_default();
    test_config_insert();
};
