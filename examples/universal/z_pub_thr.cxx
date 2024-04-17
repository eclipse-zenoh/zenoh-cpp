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
#include <stdio.h>
#include <string.h>

#include <vector>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *keyexpr = "test/thr";
    const char *payload_size = nullptr;
    const char *locator = nullptr;
    const char *configfile = nullptr;

    getargs(argc, argv, {{"payload_size", &payload_size}}, {{"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &configfile}}}
#endif
    );

    size_t len = atoi(payload_size);
    std::vector<char> payload(len, 1);

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (configfile) {
        config = expect(config_from_file(configfile));
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

    printf("Opening session...\n");
    auto session = expect<Session>(open(std::move(config)));

    PublisherOptions options;
    options.set_congestion_control(Z_CONGESTION_CONTROL_BLOCK);

    printf("Declaring Publisher on '%s'...\n", keyexpr);
    auto pub = expect<Publisher>(session.declare_publisher(keyexpr, options));

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(payload);
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
