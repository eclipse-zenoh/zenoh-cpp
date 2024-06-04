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
#include <iostream>

#include "../getargs.h"
#include "stdio.h"
#include "zenoh.hxx"
using namespace zenoh;

#ifdef ZENOHCXX_ZENOHC
const char *default_value = "Put from C++ zenoh-c!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-put";
#elif ZENOHCXX_ZENOHPICO
const char *default_value = "Put from C++ zenoh-pico!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-pico-put";
#else
#error "Unknown zenoh backend"
#endif

int _main(int argc, char **argv) {
    const char *keyexpr = default_keyexpr;
    const char *value = default_value;
    const char *locator = nullptr;
    const char *config_file = nullptr;

    getargs(argc, argv, {}, {{"key expression", &keyexpr}, {"value", &value}, {"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
    }
#endif

    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str()))
#elif ZENOHCXX_ZENOHPICO
        if (!config.insert(Z_CONFIG_CONNECT_KEY, locator))
#else
#error "Unknown zenoh backend"
#endif
        {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Putting Data (" << "'" << keyexpr << "': '" << value << "')...\n";

    std::unordered_map<std::string, std::string> attachment_map = {
        {"serial_number", "123"},
        {"coordinates", "48.7082,2.1498"}
    };

    session.put(
        KeyExpr(keyexpr), Bytes::serialize(value), {.encoding = Encoding("text/plain"), .attachment = Bytes::serialize(attachment_map) }
    );

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
