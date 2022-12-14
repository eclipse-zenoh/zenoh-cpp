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

#include <iostream>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "zenohcpp.h"

using namespace zenoh;

const char *expr = "demo/example/zenoh-cpp-queryable";
const char *value = "Queryable from CPP!";

int _main(int argc, char **argv) {
    if (argc > 1) {
        expr = argv[1];
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
    auto session = std::get<Session>(open(std::move(config)));

    KeyExprView keyexpr(expr);
    if (!keyexpr.check()) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }

    printf("Declaring Queryable on '%s'...\n", expr);

    auto queryable = std::get<Queryable>(session.declare_queryable(keyexpr, [](const Query *query) {
        if (query) {
            auto keystr = query->get_keyexpr();
            auto pred = query->get_parameters();
            std::cout << ">> [Queryable ] Received Query '" << keystr.as_string_view() << "?"
                      << pred.as_string_view() << "'\n";
            QueryReplyOptions options;
            options.set_encoding(Encoding(Z_ENCODING_PREFIX_TEXT_PLAIN));
            query->reply(expr, value, options);
        } else {
            std::cout << "Destroying queryable\n";
        }
    }));

    printf("Enter 'q' to quit...\n");
    char c = 0;
    while (c != 'q') {
        c = getchar();
        if (c == -1) {
            sleep(1);
        }
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