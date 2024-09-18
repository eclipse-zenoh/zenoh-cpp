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

#include <chrono>
#include <iostream>
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;
using namespace std::chrono_literals;

const char *expr = "demo/example/zenoh-cpp-zenoh-c-queryable";
const char *value = "Queryable from C++ zenoh-c SHM!";

const char *locator = nullptr;

int _main(int argc, char **argv) {
    const char *config_file = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}, {"locator", &locator}},
            {{"-c", {"config file", &config_file}}});

    Config config = Config::create_default();
    if (config_file) {
        config = Config::from_file(config_file);
    }

    ZResult err;
    if (locator) {
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        config.insert_json5(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str(), &err);

        if (err != Z_OK) {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = Session::open(std::move(config));

    KeyExpr keyexpr(expr);

    std::cout << "Declaring Queryable on '" << expr << "'...\n";

    std::cout << "Preparing SHM Provider...\n";
    PosixShmProvider provider(MemoryLayout(65536, AllocAlignment({0})));

    auto on_query = [provider = std::move(provider)](const Query &query) {
        auto payload = query.get_payload();

        const char *payload_type = "";
        if (payload.has_value()) {
            ZResult result;
            payload->get().deserialize<ZShm>(&result);
            if (result == Z_OK) {
                payload_type = "SHM";
            } else {
                payload_type = "RAW";
            }
        }

        const KeyExpr &keyexpr = query.get_keyexpr();
        auto params = query.get_parameters();
        std::cout << ">> [Queryable ] Received Query [" << payload_type << "] '" << keyexpr.as_string_view() << "?"
                  << params;
        if (payload.has_value()) {
            std::cout << "' value = '" << payload->get().deserialize<std::string>();
        }
        std::cout << "'\n";

        const auto len = strlen(value) + 1;  // + NULL terminator
        auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
        ZShmMut &&buf = std::get<ZShmMut>(std::move(alloc_result));
        memcpy(buf.data(), value, len);

#if __cpp_designated_initializers >= 201707L
        query.reply(KeyExpr(expr), Bytes::serialize(std::move(buf)), {.encoding = Encoding("text/plain")});
#else
        Query::ReplyOptions options;
        options.encoding = Encoding("text/plain");
        query.reply(KeyExpr(expr), Bytes::serialize(std::move(buf)), std::move(options));
#endif
    };

    auto on_drop_queryable = []() { std::cout << "Destroying queryable\n"; };

    auto queryable = session.declare_queryable(keyexpr, std::move(on_query), std::move(on_drop_queryable));

    std::cout << "Press CTRL-C to quit...\n";
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