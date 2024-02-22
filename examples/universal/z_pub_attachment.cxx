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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#undef min
#undef max
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

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
    const char *configfile = nullptr;

    getargs(argc, argv, {}, {{"key expression", &keyexpr}, {"value", &value}}
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

    std::cout << "Opening session..." << std::endl;
    auto session = expect<Session>(open(std::move(config)));

    std::cout << "Declaring Publisher on '" << keyexpr << "'..." << std::endl;
    auto pub = expect<Publisher>(session.declare_publisher(keyexpr));
#ifdef ZENOHCXX_ZENOHC
    std::cout << "Publisher on '" << pub.get_keyexpr().as_string_view() << "' declared" << std::endl;
#endif

    PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
#ifdef ZENOHCXX_ZENOHC
    // allocate attachment map
    std::map<std::string, std::string> amap;
    // set it as an attachment
    options.set_attachment(amap);
    // add some value
    amap.insert(std::pair("source", "C++"));
#endif
    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        sleep(1);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();  // in C++20 use .view() instead
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
#ifdef ZENOHCXX_ZENOHC
        // add some other attachment value
        amap["index"] = std::to_string(idx);
#endif
        pub.put(s, options);
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
