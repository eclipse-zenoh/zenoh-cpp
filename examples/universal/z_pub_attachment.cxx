//
// Copyright (c) 2023 ZettaScale Technology
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
#include <thread>
#include <chrono>

#include "../getargs.h"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#ifdef ZENOHCXX_ZENOHC
const char *default_value = "Pub from C++ zenoh-c!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-pub";
#elif ZENOHCXX_ZENOHPICO
const char *default_value = "Pub from C++ zenoh-pico!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-pico-pub";
#else
#error "Unknown zenoh backend"
#endif

int _main(int argc, char **argv) {
    const char *keyexpr = default_keyexpr;
    const char *value = default_value;
    const char *config_file = nullptr;

    getargs(argc, argv, {}, {{"key expression", &keyexpr}, {"value", &value}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    Config config = Config::create_default();
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
    }
#endif

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on '" << keyexpr << "'..." << std::endl;
    auto pub = session.declare_publisher(KeyExpr(keyexpr));

    std::cout << "Publisher on '" << keyexpr << "' declared" << std::endl;

    // allocate attachment map
    std::unordered_map<std::string, std::string> attachment_map = {
        {"source", "C++"}
    };

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();  // in C++20 use .view() instead
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
        // add some other attachment value
        attachment_map["index"] = std::to_string(idx);
        pub.put(Bytes::serialize(s), {.encoding = Encoding("text/plain"), .attachment = Bytes::serialize(attachment_map)});
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
