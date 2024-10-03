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

#include <numeric>
#include <vector>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *keyexpr = "test/thr";
    const char *payload_size = nullptr;
    Config config = parse_args(argc, argv, {{"payload_size", &payload_size}}, {{"key_expression", &keyexpr}});
    size_t len = atoi(payload_size);

    std::vector<uint8_t> data(len);
    std::iota(data.begin(), data.end(), uint8_t{0});
    Bytes payload = std::move(data);

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on " << keyexpr << "...\n";
#if __cpp_designated_initializers >= 201707L
    auto pub = session.declare_publisher(KeyExpr(keyexpr), {.congestion_control = Z_CONGESTION_CONTROL_BLOCK});
#else
    auto pub_options = Session::PublisherOptions::create_default();
    pub_options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
    auto pub = session.declare_publisher(KeyExpr(keyexpr), std::move(pub_options));
#endif

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(payload.clone());
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
