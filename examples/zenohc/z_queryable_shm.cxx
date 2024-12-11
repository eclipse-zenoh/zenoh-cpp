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

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;
using namespace std::chrono_literals;

const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-queryable";
const char *default_payload = "Queryable from C++ zenoh-c SHM!";

const char *locator = nullptr;

int _main(int argc, char **argv) {
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"k", "key"}, "KEY_EXPRESSION", "Key expression matching queries to reply to (string)",
                         default_keyexpr)
            .named_value({"p", "payload"}, "PAYLOAD", "Value to reply to queries with (string)", default_payload)
            .named_flag({"complete"}, "Flag to indicate whether queryable is complete or not")
            .run();

    auto keyexpr = args.value("key");
    auto payload = args.value("payload");
    auto complete = args.flag("complete");

    printf("Opening session...\n");
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Queryable on '" << keyexpr << "'...\n";

    std::cout << "Preparing SHM Provider...\n";
    PosixShmProvider provider(MemoryLayout(65536, AllocAlignment({0})));

    auto on_query = [provider = std::move(provider), payload, keyexpr](const Query &query) {
        auto query_payload = query.get_payload();

        const char *payload_type = "";
        if (query_payload.has_value()) {
            ZResult result;
            query_payload->get().as_shm(&result);
            if (result == Z_OK) {
                payload_type = "SHM";
            } else {
                payload_type = "RAW";
            }
        }

        auto params = query.get_parameters();
        std::cout << ">> [Queryable ] Received Query [" << payload_type << "] '" << query.get_keyexpr().as_string_view()
                  << "?" << params;
        if (query_payload.has_value()) {
            std::cout << "' value = '" << query_payload->get().as_string();
        }
        std::cout << "'\n";

        const auto len = payload.size() + 1;  // + NULL terminator
        auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
        ZShmMut &&buf = std::get<ZShmMut>(std::move(alloc_result));
        memcpy(buf.data(), payload.data(), len);

        std::cout << "[Queryable ] Responding ('" << keyexpr << "': '" << payload << "')\n";
        query.reply(keyexpr, std::move(buf));
    };

    auto on_drop_queryable = []() { std::cout << "Destroying queryable\n"; };

    Session::QueryableOptions opts;
    opts.complete = complete;
    auto queryable =
        session.declare_queryable(keyexpr, std::move(on_query), std::move(on_drop_queryable), std::move(opts));

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