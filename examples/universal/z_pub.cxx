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

#include <iostream>
#include <limits>
#include <sstream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *keyexpr = "demo/example/zenoh-cpp-pub";
    const char *value = "Pub from CPP!";
    const char *locator = nullptr;

    if (argc > 1) keyexpr = argv[1];
    if (argc > 2) value = argv[2];
    if (argc > 3) locator = argv[3];

    Config config;
    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str()))
#elif ZENOHCXX_ZENOHPICO
        if (!config.insert(Z_CONFIG_PEER_KEY, locator))
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
    auto session = std::get<Session>(open(std::move(config)));

    printf("Declaring Publisher on '%s'...\n", keyexpr);
    auto pub = std::get<Publisher>(session.declare_publisher(keyexpr));

    PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
    for (int idx = 0; std::numeric_limits<int>::max(); ++idx) {
        sleep(1);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();  // in C++20 use .view() instead
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
        pub.put(s);
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
