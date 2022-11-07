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

#include "zenohcpp.h"

using namespace zenoh;

int main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    if (argc > 1) {
        expr = argv[1];
    }
    KeyExprRef keyexpr(expr);
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
    Session s(std::move(config));
    if (!s.check()) {
        printf("Unable to open session!\n");
        exit(-1);
    }

    printf("Sending Query '%s'...\n", expr);
    auto opts = GetOptions().set_target(Z_QUERY_TARGET_ALL);
    ReplyFIFO channel(16);
    s.get(keyexpr, "",  channel.take_send(), opts);
    Reply reply;
    for(channel.get_recv().call(reply); reply.check(); channel.get_recv().call(reply)) {
        if (reply.is_ok()) {
            auto sample = reply.ok();
            auto keystr = sample.get_keyexpr().str();
            printf(">> Received ('%s': '%.*s')\n", keystr.c_str(), (int)sample.payload.len, sample.payload.start);
        } else {
            printf("Received an error\n");
        }
    }
    return 0;
}