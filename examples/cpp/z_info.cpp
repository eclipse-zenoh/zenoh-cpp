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

#include <iostream>

#include "zenohcpp.h"

using namespace zenoh;

void print_zid(const Id& id) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", id.id[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    Config config;
    if (argc > 1) {
        if (config.insert_json(Z_CONFIG_CONNECT_KEY, argv[3]) < 0) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[1], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    auto self_id = session.info_zid();
    printf("own id: ");
    print_zid(self_id);

    printf("routers ids:\n");
    session.info_routers_zid([](const Id* id) {
        if (id) print_zid(*id);
    });

    printf("peers ids:\n");
    session.info_peers_zid([](const Id* id) {
        if (id) print_zid(*id);
    });
}
