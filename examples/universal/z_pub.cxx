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

#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

#include "../getargs.hxx"
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
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"k", "key"}, "KEY_EXPRESSION", "Key expression to publish to (string)", default_keyexpr)
            .named_value({"p", "payload"}, "PAYLOAD", "Payload to publish (string)", default_value)
            .named_value({"a", "attach"}, "ATTACHMENT", "Attachment to add to each put (string)", "")
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
            .named_flag({"add-matching-listener"}, "Add matching listener")
#endif
            .run();

    auto keyexpr = args.value("key");
    auto payload = args.value("payload");
    auto attachment = args.value("attach");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on '" << keyexpr << "'..." << std::endl;
    auto pub = session.declare_publisher(KeyExpr(keyexpr));
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
    if (args.flag("add-matching-listener")) {
        pub.declare_background_matching_listener(
            [](const MatchingStatus &s) {
                if (s.matching) {
                    std::cout << "Publisher has matching subscribers." << std::endl;
                } else {
                    std::cout << "Publisher has NO MORE matching subscribers." << std::endl;
                }
            },
            closures::none);
    }
#endif

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << payload;
        auto s = ss.str();
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
        Publisher::PutOptions options;
        if (!attachment.empty()) {
            options.attachment = attachment;
        }

        pub.put(s, std::move(options));
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
