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

void test_config_client() {
    std::vector<std::string> peers = {"tcp/192.168.0.1", "tcp/10.0.0.1"};
    auto config = Config::client(peers);
    assert(config.get("mode") == "\"client\"");
    assert(config.get("connect/endpoints") == "[\"tcp/192.168.0.1\",\"tcp/10.0.0.1\"]");
}

void test_config_peer() {
    auto config = Config::peer();
    assert(config.get("mode") == "\"peer\"");
}

void test_config_to_string() {
    Config config = Config::create_default();
    auto s = config.to_string();
    assert(s.size() > 0);
    assert(s.find("{\"id\":\"") == 0);
}

int main(int argc, char** argv) {
    init_logger();
    test_config_client();
    test_config_peer();
    test_config_to_string();
    // TODO: add more tests
};
