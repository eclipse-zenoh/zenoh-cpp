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

#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

const char *default_value = "Pub from C++ zenoh-c SHM!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-pub";

int _main(int argc, char **argv) {
    const char *keyexpr = default_keyexpr;
    const char *value = default_value;
    const char *add_matching_listener = "false";
    Config config = parse_args(argc, argv, {}, {{"key_expression", &keyexpr}, {"payload_value", &value}}
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
                               ,
                               {{"--add-matching-listener", {CmdArg{"", &add_matching_listener, true}}}}
#endif
    );

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on '" << keyexpr << "'..." << std::endl;
    auto pub = session.declare_publisher(KeyExpr(keyexpr));

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    if (std::string(add_matching_listener) == "true") {
        pub.declare_background_matching_listener(
            [](const Publisher::MatchingStatus &s) {
                if (s.matching) {
                    std::cout << "Subscriber matched" << std::endl;
                } else {
                    std::cout << "No subscribers matched" << std::endl;
                }
            },
            closures::none);
    }
#endif

    std::cout << "Publisher on '" << keyexpr << "' declared" << std::endl;

    std::cout << "Preparing SHM Provider...\n";
    PosixShmProvider provider(MemoryLayout(65536, AllocAlignment({2})));

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();  // in C++20 use .view() instead
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";

        std::cout << "Allocating SHM buffer...\n";
        const auto len = s.size() + 1;
        auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
        ZShmMut &&buf = std::get<ZShmMut>(std::move(alloc_result));
        memcpy(buf.data(), s.data(), len);

#if __cpp_designated_initializers >= 201707L
        pub.put(std::move(buf), {.encoding = Encoding("text/plain")});
#else
        Publisher::PutOptions options;
        options.encoding = Encoding("text/plain");
        pub.put(std::move(buf), std::move(options));
#endif
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
#ifdef ZENOHCXX_ZENOHC
        init_log_from_env_or("error");
#endif
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
