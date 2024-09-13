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
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;
using namespace std::chrono_literals;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *locator = nullptr;
    const char *config_file = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"locator", &locator}}
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

    ZResult err;
    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str(), &err);
#elif ZENOHCXX_ZENOHPICO
        config.insert(Z_CONFIG_CONNECT_KEY, locator, &err);
#else
#error "Unknown zenoh backend"
#endif
        if (err != Z_OK) {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    KeyExpr keyexpr(expr);
    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << expr << "'...\n";

#if __cpp_designated_initializers >= 201707L
    auto replies =
        session.get(keyexpr, "", channels::FifoChannel(16),
                    {.target = QueryTarget::Z_QUERY_TARGET_ALL, .payload = Bytes::serialize("Get from C++")});
#else
    Session::GetOptions options;
    options.target = QueryTarget::Z_QUERY_TARGET_ALL;
    options.payload = Bytes::serialize("Get from C++");
    auto replies = session.get(keyexpr, "", channels::FifoChannel(16), std::move(options));
#endif

    while (true) {
        auto res = replies.try_recv();
        if (std::holds_alternative<channels::RecvError>(res)) {
            if (std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA) {
                std::cout << ".";
                std::this_thread::sleep_for(1s);
                continue;
            } else {  // channel is closed - no more replies will be received
                break;
            }
        }
        const auto &sample = std::get<Reply>(res).get_ok();
        std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                  << sample.get_payload().deserialize<std::string>() << "')\n";
    }
    std::cout << std::endl;

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
