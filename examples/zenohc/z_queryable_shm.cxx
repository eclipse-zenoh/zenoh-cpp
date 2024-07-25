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
    config.insert_json(Z_CONFIG_SHARED_MEMORY_KEY, "true");
    if (config_file) {
        config = Config::from_file(config_file);
    }

    ZResult err;
    if (locator) {
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str(), &err);

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
        const char *payload_type = "RAW";
        {
            ZResult result;
            query.get_payload().deserialize<ZShm>(&result);
            if (result == Z_OK) {
                payload_type = "SHM";
            }
        }

        const KeyExpr &keyexpr = query.get_keyexpr();
        auto params = query.get_parameters();
        std::cout << ">> [Queryable ] Received Query [" << payload_type << "] '" << keyexpr.as_string_view() << "?"
                  << params << "' value = '" << query.get_payload().deserialize<std::string>() << "'\n";

        const auto len = strlen(value) + 1;  // + NULL terminator
        auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
        ZShmMut &&buf = std::get<ZShmMut>(std::move(alloc_result));
        memcpy(buf.data(), value, len);

        query.reply(KeyExpr(expr), Bytes::serialize(std::move(buf)), {.encoding = Encoding("text/plain")});
    };

    auto on_drop_queryable = []() { std::cout << "Destroying queryable\n"; };

    auto queryable = session.declare_queryable(keyexpr, on_query, on_drop_queryable);

    std::cout << "Press CTRL-C to quit...\n";
    while (true) {
        std::this_thread::sleep_for(1s);
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}