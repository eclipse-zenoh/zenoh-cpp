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
#include <chrono>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

#define N 1000000

struct Stats {
    volatile unsigned long count = 0;
    volatile unsigned long finished_rounds = 0;
    std::chrono::steady_clock::time_point start = {};
    std::chrono::steady_clock::time_point first_start = {};
    std::chrono::steady_clock::time_point end = {};

    void operator()(const Sample &) {
        if (count == 0) {
            start = std::chrono::steady_clock::now();
            if (first_start == std::chrono::steady_clock::time_point()) {
                first_start = start;
            }
            count++;
        } else if (count < N) {
            count++;
        } else {
            finished_rounds++;
            auto elapsed_us =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << static_cast<double>(N) * 1000000.0 / static_cast<double>(elapsed_us) << " msg/s\n";
            count = 0;
        }
    }

    void operator()() { end = std::chrono::steady_clock::now(); }

    void print() const {
        auto elapsed_s =
            static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - first_start).count()) /
            1000000.0;
        const unsigned long sent_messages = N * finished_rounds + count;
        std::cout << "Sent " << sent_messages << " messages over " << elapsed_s << " seconds ("
                  << static_cast<double>(sent_messages) / elapsed_s << " msg/s)\n";
    }
};

int _main(int argc, char **argv) {
    const char *locator = nullptr;
    const char *configfile = nullptr;

    getargs(argc, argv, {}, {{"locator", &locator}}
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

    KeyExpr keyexpr = session.declare_keyexpr("test/thr");

    Stats stats;
    auto subscriber = expect<Subscriber>(session.declare_subscriber(keyexpr, {stats, stats}));

    printf("Press CTRL-C to quit...\n");
    while (1) {
        sleep(1);
    }

    subscriber.drop();
    stats.print();

    session.undeclare_keyexpr(std::move(keyexpr));

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
