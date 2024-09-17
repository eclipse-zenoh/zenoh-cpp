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
//
#include <stdio.h>
#include <string.h>

#include <vector>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *keyexpr = "test/thr";
    const char *payload_size = nullptr;
    const char *locator = nullptr;
    const char *config_file = nullptr;

    getargs(argc, argv, {{"payload_size", &payload_size}}, {{"locator", &locator}},
            {{"-c", {"config file", &config_file}}});

    size_t len = atoi(payload_size);

    Config config = Config::create_default();
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

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on " << keyexpr << "...\n";

#if __cpp_designated_initializers >= 201707L
    auto pub = session.declare_publisher(KeyExpr(keyexpr), {.congestion_control = Z_CONGESTION_CONTROL_BLOCK});
#else
    Session::PublisherOptions options;
    options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
    auto pub = session.declare_publisher(keyexpr, std::move(options));
#endif
    std::cout << "Preparing SHM Provider...\n";
    constexpr auto buffers_count = 4;
    PosixShmProvider provider(MemoryLayout(buffers_count * len, AllocAlignment({2})));

    std::cout << "Allocating SHM buffer...\n";
    auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
    ZShmMut &&buf_mut = std::get<ZShmMut>(std::move(alloc_result));
    ZShm buf(std::move(buf_mut));

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(Bytes::serialize(ZShm(buf)));
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
