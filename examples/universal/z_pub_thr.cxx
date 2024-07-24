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
    const char *config_file = nullptr;

    getargs(argc, argv, {{"payload_size", &payload_size}}, {{"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    size_t len = atoi(payload_size);
    std::vector<uint8_t> payload(len, 1);

    Config config = Config::create_default();
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
    }
#endif
    ZResult err;
    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str(), &err);
#elif ZENOHCXX_ZENOHPICO
        config.insert(Z_CONFIG_CONNECT_KEY, locator, &err);
#else
#error "Unknown zenoh backend"
#endif
        if (err != Z_OK) {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on " << keyexpr << "...\n";
    auto pub = session.declare_publisher(KeyExpr(keyexpr), {.congestion_control = Z_CONGESTION_CONTROL_BLOCK});

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(Bytes::serialize(payload, ZenohCodec<ZenohCodecType::AVOID_COPY>()));
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
