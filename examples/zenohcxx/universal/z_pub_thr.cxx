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

#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    if (argc < 2) {
        printf("USAGE:\n\tz_pub_thr <payload-size> [<zenoh-locator>]\n\n");
        exit(-1);
    }

    const char *keyexpr = "test/thr";
    size_t len = atoi(argv[1]);
    std::vector<char> payload(len, 1);

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (argc > 2) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[2])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[2], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }
#endif

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    PublisherOptions options;
    options.set_congestion_control(Z_CONGESTION_CONTROL_BLOCK);

    printf("Declaring Publisher on '%s'...\n", keyexpr);
    auto pub = std::get<Publisher>(session.declare_publisher(keyexpr, options));

    while (1) pub.put(payload);
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
