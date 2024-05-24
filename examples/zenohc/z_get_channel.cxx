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
#include <string>
#include <iostream>

#include "zenoh.hxx"
#include "serde.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    if (argc > 1) {
        expr = argv[1];
    }
    KeyExpr keyexpr(expr);
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
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << expr << "'...\n";
    auto replies = session.get_reply_fifo_channel<FifoChannelType::Blocking>(
        keyexpr, "", 16, {.target = QueryTarget::Z_QUERY_TARGET_ALL}
    );

    for (auto reply = replies.get_next_reply(); replies.is_active() && reply; reply = replies.get_next_reply()) {
        const auto& sample = reply.get_ok();
        std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                  << serde::deserialize<std::string>(sample.get_payload())<< "')\n";
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
