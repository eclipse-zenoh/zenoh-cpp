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

    if (argc > 1) keyexpr = argv[1];
    if (argc > 2) value = argv[2];

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (argc > 3) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[3])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[3], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }
#endif

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
