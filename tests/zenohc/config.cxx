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

#include "zenohc.hxx"

using namespace zenohc;

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

void test_config_to_string() {
    Config config;
    auto s = config.to_string();
    std::string_view sv(s);
    assert(sv.length() > 0);
    assert(sv.find("{\"id\":\"") == 0);
}

void test_config_from_file() {
    auto config = config_from_file("BAD FILE");
    assert(std::get_if<Config>(&config) == nullptr);
    assert(std::get_if<ErrorMessage>(&config) != nullptr);
}

void test_config_from_str() {
    auto config = config_from_str("BAD STR");
    assert(std::get_if<Config>(&config) == nullptr);
    assert(std::get_if<ErrorMessage>(&config) != nullptr);
}

int main(int argc, char** argv) {
    init_logger();
    test_config_client();
    test_config_peer();
    test_config_to_string();
    test_config_from_file();
    test_config_from_str();
};
