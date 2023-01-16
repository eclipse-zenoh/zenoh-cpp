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
#include <time.h>

#include "zenohcpp.h"

using namespace zenoh;

#define N 1000000

struct Stats {
    volatile unsigned long count = 0;
    volatile unsigned long finished_rounds = 0;
    volatile clock_t start = 0;
    volatile clock_t stop = 0;
    volatile clock_t end = 0;
    volatile clock_t first_start = 0;

    void operator()(const Sample *sample) {
        if (sample) {
            if (count == 0) {
                start = clock();
                if (!first_start) {
                    first_start = start;
                }
                count++;
            } else if (count < N) {
                count++;
            } else {
                stop = clock();
                finished_rounds++;
                printf("%f msg/s\n", N * (double)CLOCKS_PER_SEC / (double)(stop - start));
                count = 0;
            }
        } else {
            end = clock();
        }
    }

    void print() const {
        const double elapsed = (double)(end - first_start) / (double)CLOCKS_PER_SEC;
        const unsigned long sent_messages = N * finished_rounds + count;
        printf("Sent %ld messages over %f seconds (%f msg/s)\n", sent_messages, elapsed,
               (double)sent_messages / elapsed);
    }
};

int _main(int argc, char **argv) {
    Config config;
    if (argc > 1) {
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, argv[1])) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[1], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    KeyExpr keyexpr = session.declare_keyexpr("test/thr");

    Stats stats;
    auto subscriber = std::get<Subscriber>(session.declare_subscriber(keyexpr, stats));
    char c = 0;
    while (c != 'q') {
        c = fgetc(stdin);
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
