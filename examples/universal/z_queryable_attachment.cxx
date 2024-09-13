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

#include <stdio.h>
#include <string.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#ifdef ZENOHCXX_ZENOHC
const char *expr = "demo/example/zenoh-cpp-zenoh-c-queryable";
const char *value = "Queryable from C++ zenoh-c!";
#elif ZENOHCXX_ZENOHPICO
const char *expr = "demo/example/zenoh-cpp-zenoh-pico-queryable";
const char *value = "Queryable from C++ zenoh-pico!";
#else
#error "Unknown zenoh backend"
#endif

int _main(int argc, char **argv) {
    const char *config_file = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}}
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

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    KeyExpr keyexpr(expr);

    std::cout << "Declaring Queryable on '" << expr << "'...\n";

    auto on_query = [](const Query &query) {
        const KeyExpr &keyexpr = query.get_keyexpr();
        auto params = query.get_parameters();
        auto payload = query.get_payload();
        std::cout << ">> [Queryable ] Received Query '" << keyexpr.as_string_view() << "?" << params;
        if (payload.has_value()) {
            std::cout << "' value = '" << payload->get().deserialize<std::string>();
        }
        std::cout << "'\n";

        std::unordered_map<std::string, std::string> attachment_map;
        auto attachment = query.get_attachment();
        if (attachment.has_value()) {
            // read attachment as a key-value map
            attachment_map = attachment->get().deserialize<std::unordered_map<std::string, std::string>>();
            for (auto &&[key, value] : attachment_map) {
                std::cout << "   attachment: " << key << ": '" << value << "'\n";
            }
        }
#if __cpp_designated_initializers >= 201707L
        query.reply(KeyExpr(expr), Bytes::serialize(value),
                    {.encoding = Encoding("text/palin"), .attachment = Bytes::serialize(attachment_map)});
#else
        Query::ReplyOptions options;
        options.encoding = Encoding("text/plain");
        options.attachment = Bytes::serialize(attachment_map);
        query.reply(KeyExpr(expr), Bytes::serialize(value), std::move(options));
#endif
    };

    auto on_drop_queryable = []() { std::cout << "Destroying queryable\n"; };

    auto queryable = session.declare_queryable(keyexpr, on_query, on_drop_queryable);

    printf("Press CTRL-C to quit...\n");
    while (true) {
        std::this_thread::sleep_for(1s);
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
