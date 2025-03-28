//
// Copyright (c) 2024 ZettaScale Technology
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
#include <chrono>
#include <cstdlib>
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
            .named_value({"i", "history"}, "HISTORY_SIZE", "The number of publications to keep in cache (number)", "1")
            .run();

    config.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");
    auto keyexpr = args.value("key");
    auto payload = args.value("payload");
    auto history = std::atoi(args.value("history").data());

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    ext::SessionExt::AdvancedPublisherOptions opts;
    opts.cache.emplace().max_samples = history;
    opts.publisher_detection = true;
    opts.sample_miss_detection.emplace().heartbeat_period_ms = 500;
    opts.sample_miss_detection.emplace().heartbeat_mode = ext::SessionExt::AdvancedPublisherOptions::HeartbeatMode::PERIODIC;
    // alternatively sample miss detection can be done in response to subscriber's periodic queries:
    // opts.sample_miss_detection.emplace();

    std::cout << "Declaring AdvancedPublisher on '" << keyexpr << "'..." << std::endl;
    auto pub = session.ext().declare_advanced_publisher(KeyExpr(keyexpr), std::move(opts));

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << payload;
        auto s = ss.str();
        std::cout << "Put Data ('" << keyexpr << "': '" << s << "')...\n";
        pub.put(s);
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
