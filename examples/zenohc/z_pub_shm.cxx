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
    auto session = expect<Session>(open(std::move(config)));
    std::ostringstream oss;
    oss << session.info_zid();

    auto manager = expect<ShmManager>(shm_manager_new(session, oss.str().c_str(), N * 256));

    printf("Declaring Publisher on '%s'...\n", keyexpr);
    auto pub = expect<Publisher>(session.declare_publisher(keyexpr));

    PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
    for (int idx = 0; idx < N; ++idx) {
        auto shmbuf = expect<z::Shmbuf>(manager.alloc(256));
        auto buf = shmbuf.char_ptr();
        snprintf(buf, 255, "[%4d] %s", idx, value);
        shmbuf.set_length(strlen(buf));
        sleep(1);
        std::cout << "Putting Data ('" << keyexpr << "': '" << shmbuf.as_string_view() << "')..." << std::endl;
        auto payload = shmbuf.into_payload();
        pub.put_owned(std::move(payload), options);
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
