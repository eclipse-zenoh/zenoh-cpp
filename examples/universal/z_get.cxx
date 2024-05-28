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
#include <map>
#include <mutex>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *value = nullptr;
    const char *locator = nullptr;
    const char *config_file = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}, {"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
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

    KeyExpr keyexpr(expr);

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << expr << "'...\n";

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_reply = [](const Reply &reply) {
        if (reply.is_ok()) {
            const auto& sample = reply.get_ok();
            std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                      << sample.get_payload().deserialize<std::string>() << "')\n";
        } else {
            std::cout << "Received an error :" << reply.get_err().get_payload().deserialize<std::string>() << "\n";
        }
    };

    auto on_done = [&m, &done, &done_signal]() {
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    session.get(keyexpr, "", make_scoped_closure(std::move(on_reply), std::move(on_done)), {.target = Z_QUERY_TARGET_ALL});

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
