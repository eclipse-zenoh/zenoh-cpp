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
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif
#include <iostream>

#include "zenohcpp.h"

using namespace zenoh;

const char *kind_to_str(z_sample_kind_t kind);

void data_handler(const Sample *sample) {
    if (sample) {
        std::cout << ">> [Subscriber] Received " << kind_to_str(sample->get_kind()) << " ('"
                  << sample->get_keyexpr().as_string_view() << "' : '" << sample->get_payload().as_string_view()
                  << "')\n";
    }
}

int main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    if (argc > 1) {
        expr = argv[1];
    }
    KeyExprView keyexpr(expr);

    Config config;
    if (argc > 2) {
        if (!config.insert_json(Z_CONFIG_LISTEN_KEY, argv[2])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[2], Z_CONFIG_LISTEN_KEY, Z_CONFIG_LISTEN_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    auto callback = ClosureSample(data_handler);

    printf("Declaring Subscriber on '%s'...\n", expr);
    auto subscriber = std::get<Subscriber>(session.declare_subscriber(keyexpr, std::move(callback)));

    printf("Enter 'q' to quit...\n");
    char c = 0;
    while (c != 'q') {
        c = getchar();
        if (c == -1) {
            sleep(1);
        }
    }

    return 0;
}

const char *kind_to_str(z_sample_kind_t kind) {
    switch (kind) {
        case Z_SAMPLE_KIND_PUT:
            return "PUT";
        case Z_SAMPLE_KIND_DELETE:
            return "DELETE";
        default:
            return "UNKNOWN";
    }
}