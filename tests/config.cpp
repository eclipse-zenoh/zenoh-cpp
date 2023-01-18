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

void test_config_client() {
    std::vector<const char*> peers = {"tcp/192.168.0.1", "tcp/10.0.0.1"};
    auto config = config_client(peers);
    assert(std::get_if<Config>(&config) != nullptr);

    auto config1 = config_client({"tcp/192.168.0.1", "tcp/10.0.0.1"});
    assert(std::get_if<Config>(&config1) != nullptr);
}

void test_config_peer() {
    auto config = config_peer();
    assert(config.check());
    assert(config.get("mode") == "\"peer\"");
}

int main(int argc, char** argv) {
    init_logger();
    test_config_client();
    test_config_peer();
};
