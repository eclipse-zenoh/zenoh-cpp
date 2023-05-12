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

#include <condition_variable>
#include <iostream>

#include "zenohc.hxx"
using namespace zenohc;

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

#include "zenoh.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#define N 10

int _main(int argc, char **argv) {
    const char *keyexpr = "demo/example/zenoh-cpp-pub-shm";
    const char *value = "Pub from CPP!";

    if (argc > 1) keyexpr = argv[1];
    if (argc > 2) value = argv[2];

    z_owned_config_t config = z_config_default();
    if (argc > 3) {
        if (zc_config_insert_json(z_loan(config), Z_CONFIG_CONNECT_KEY, argv[3]) < 0) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[3], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));
    std::ostringstream oss;
    oss << session.info_zid();

    ShmManager manager(session, oss.str().c_str(), N * 256);

    // zc_owned_shm_manager_t manager = zc_shm_manager_new(z_loan(s), idstr, N * 256);
    // if (!z_check(s)) {
    //     printf("Unable to open session!\n");
    //     exit(-1);
    // }

    // printf("Declaring Publisher on '%s'...\n", keyexpr);
    // z_owned_publisher_t pub = z_declare_publisher(z_loan(s), z_keyexpr(keyexpr), NULL);
    // if (!z_check(pub)) {
    //     printf("Unable to declare Publisher for key expression!\n");
    //     exit(-1);
    // }

    // char buf[256];
    // for (int idx = 0; idx < N; ++idx) {
    //     zc_owned_shmbuf_t shmbuf = zc_shm_alloc(&manager, 256);
    //     if (!z_check(shmbuf)) {
    //         printf("Failed to allocate a SHM buffer\n");
    //         exit(-1);
    //     }
    //     uint8_t *buf = zc_shmbuf_ptr(&shmbuf);
    //     sleep(1);
    //     sprintf((char *)buf, "[%4d] %s", idx, value);
    //     printf("Putting Data ('%s': '%s')...\n", keyexpr, buf);
    //     z_publisher_put_options_t options = z_publisher_put_options_default();
    //     options.encoding = z_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN, NULL);
    //     zc_owned_payload_t payload = zc_shmbuf_into_payload(z_move(shmbuf));
    //     zc_publisher_put_owned(z_loan(pub), z_move(payload), &options);
    // }

    // z_undeclare_publisher(z_move(pub));

    // z_close(z_move(s));
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
