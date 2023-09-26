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

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

const char *kind_to_str(z_sample_kind_t kind);

void data_handler(const Sample &sample) {
    std::cout << ">> [Subscriber] Received " << kind_to_str(sample.get_kind()) << " ('"
              << sample.get_keyexpr().as_string_view() << "' : '" << sample.get_payload().as_string_view() << "')\n";
}

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *locator = nullptr;
    const char *configfile = nullptr;

    getargs(argc, argv, {}, {{"key expression", &expr}, {"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &configfile}}}
#endif
    );

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

    KeyExprView keyexpr(expr);

    std::cout << "Opening session..." << std::endl;
    auto session = expect<Session>(open(std::move(config)));

    std::cout << "Declaring Subscriber on '" << keyexpr.as_string_view() << "'..." << std::endl;
    auto subscriber = expect<Subscriber>(session.declare_subscriber(keyexpr, data_handler));
#ifdef ZENOHCXX_ZENOHC
    std::cout << "Subscriber on '" << subscriber.get_keyexpr().as_string_view() << "' declared" << std::endl;
#endif

    printf("Enter 'q' to quit...\n");
    int c = 0;
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

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}