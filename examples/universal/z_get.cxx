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

#include <stdio.h>
#include <string.h>

#include <condition_variable>
#include <iostream>
#include <mutex>

#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *value = nullptr;
    const char *locator = nullptr;

    if (argc > 1) expr = argv[1];
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

    KeyExprView keyexpr(expr);
    if (!keyexpr.check()) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    std::cout << "Sending Query '" << expr << "'...\n";
    GetOptions opts;
    opts.set_target(Z_QUERY_TARGET_ALL);
    opts.set_value(value);

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    session.get(
        keyexpr, "",
        [&m, &done, &done_signal](Reply reply) {
            if (reply.check()) {
                auto result = reply.get();
                if (auto sample = std::get_if<Sample>(&result)) {
                    std::cout << "Received ('" << sample->get_keyexpr().as_string_view() << "' : '"
                              << sample->get_payload().as_string_view() << "')\n";
                } else if (auto error = std::get_if<ErrorMessage>(&result)) {
                    std::cout << "Received an error :" << error->as_string_view() << "\n";
                }
            } else {
                // No more replies
                std::lock_guard lock(m);
                done = true;
                done_signal.notify_all();
            }
        },
        opts);

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
