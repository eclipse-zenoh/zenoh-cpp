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

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

#ifdef ZENOHCXX_ZENOHC
const char *expr = "demo/example/zenoh-cpp-zenoh-c-queryable";
const char *value = "Queryable from C++ zenoh-c!";
#elif ZENOHCXX_ZENOHPICO
const char *expr = "demo/example/zenoh-cpp-zenoh-pico-queryable";
const char *value = "Queryable from C++ zenoh-pico!";
#else
#error "Unknown zenoh backend"
#endif
const char *locator = nullptr;

int _main(int argc, char **argv) {
    const char *configfile = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}, {"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &configfile}}}
#endif
    );

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (configfile) {
        config = expect(config_from_file(configfile));
    }
#endif

    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str()))
#elif ZENOHCXX_ZENOHPICO
        if (!config.insert(Z_CONFIG_CONNECT_KEY, locator))
#else
#error "Unknown zenoh backend"
#endif
        {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = expect<Session>(open(std::move(config)));

    KeyExprView keyexpr(expr);
    if (!keyexpr.check()) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }

    printf("Declaring Queryable on '%s'...\n", expr);

    auto on_query = [](const Query &query) {
        auto keystr = query.get_keyexpr();
        auto pred = query.get_parameters();
        auto query_value = query.get_value();
        std::cout << ">> [Queryable ] Received Query '" << keystr.as_string_view() << "?" << pred.as_string_view()
                  << "' value = '" << query_value.as_string_view() << "'\n";
        QueryReplyOptions options;
        options.set_encoding(Encoding(Z_ENCODING_PREFIX_TEXT_PLAIN));
        query.reply(expr, value, options);
    };

    auto on_drop_queryable = []() { std::cout << "Destroying queryable\n"; };

    auto queryable = expect<Queryable>(session.declare_queryable(keyexpr, {on_query, on_drop_queryable}));

    printf("Enter 'q' to quit...\n");
    int c = 0;
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