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

#include "zenoh.hxx"

#if defined(ZENOHCXX_ZENOHPICO)
using namespace zenohpico;
#elif defined(ZENOHCXX_ZENOHC)
using namespace zenohc;
#endif

void print_zid(const Id* id) {
    if (id) std::cout << id << std::endl;
}

int _main(int argc, char** argv) {
    Config config;
    if (argc > 1) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[3])) {
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
    print_zid(&self_id);

    printf("routers ids:\n");
    session.info_routers_zid(print_zid);

    printf("peers ids:\n");
    session.info_peers_zid(print_zid);
    return 0;
}

int main(int argc, char** argv) {
    try {
        return _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
