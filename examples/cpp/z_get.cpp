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

#include <iostream>
#include <mutex>
#include <condition_variable>

#include "zenohcpp.h"

using namespace zenoh;

int main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    if (argc > 1) {
        expr = argv[1];
    }
    KeyExprView keyexpr(expr);
    if (!keyexpr.check()) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }
    Config config;
    if (argc > 2) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[2])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[2], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    std::cout << "Sending Query '" << expr << "'...\n";
    GetOptions opts;
    opts.set_target(Z_QUERY_TARGET_ALL);

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    session.get(keyexpr, "", [&m, &done, &done_signal](std::optional<Reply> reply) {
        if (reply.has_value()) {
            auto result = reply->get();
            if (auto sample = std::get_if<Sample>(&result)) {
                std::cout 
                    << "Received ('" 
                    << sample->get_keyexpr().as_string_view()
                    << "' : '" 
                    << sample->get_payload().as_string_view() 
                    << "')\n";
            } else if (auto error = std::get_if<ErrorMessage>(&result)) {
                std::cout << "Received an error :" << error->as_string_view() << "\n";
            }
        } else {
            std::cout << "No more replies\n";
            std::lock_guard lock(m);
            done = true;
            done_signal.notify_all();
        }
    }, opts);

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] {return done;});

    return 0;
}