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
#include <thread>

#include "../getargs.hxx"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

struct Stats {
    volatile unsigned long count = 0;
    volatile unsigned long finished_rounds = 0;
    std::chrono::high_resolution_clock::time_point start = {};
    std::chrono::high_resolution_clock::time_point first_start = {};
    std::chrono::high_resolution_clock::time_point end = {};
    size_t max_rounds;
    size_t messages_per_round;

    Stats(size_t samples, size_t messages) : max_rounds(samples), messages_per_round(messages) {}

    void operator()(const Sample &) {
        if (count == 0) {
            start = std::chrono::high_resolution_clock::now();
            if (first_start == std::chrono::high_resolution_clock::time_point()) {
                first_start = start;
            }
            count++;
        } else if (count < messages_per_round) {
            count++;
        } else {
            finished_rounds++;
            auto elapsed_us =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start)
                    .count();
            std::cout << static_cast<double>(messages_per_round) * 1000000.0 / static_cast<double>(elapsed_us)
                      << " msg/s\n";
            count = 0;
            if (finished_rounds > max_rounds) {
                std::exit(0);
            }
        }
    }

    void operator()() {
        end = std::chrono::high_resolution_clock::now();
        print();
    }

    void print() const {
        auto elapsed_s =
            static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - first_start).count()) /
            1000000.0;
        const unsigned long sent_messages = messages_per_round * finished_rounds + count;
        std::cout << "Sent " << sent_messages << " messages over " << elapsed_s << " seconds ("
                  << static_cast<double>(sent_messages) / elapsed_s << " msg/s)\n";
    }
};

int _main(int argc, char **argv) {
    auto &&[config, args] = ConfigCliArgParser(argc, argv)
                                .named_value({"s", "samples"}, "MESUREMENTS", "Number of throughput measurements", "10")
                                .named_value({"n", "number"}, "NUM_MESSAGES",
                                             "Number of messages in each throughput measurement", "1000000")
                                .run();

    auto samples = std::atoi(args.value("samples").data());
    auto num_messages = std::atoi(args.value("number").data());

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    Stats stats(samples, num_messages);
    auto on_receive = [&stats](const Sample &s) { stats(s); };
    auto on_drop = [&stats]() { stats(); };
    session.declare_background_subscriber("test/thr", on_receive, on_drop);

    std::cout << "Press CTRL-C to quit...\n";
    while (true) {
        std::this_thread::sleep_for(1s);
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
#ifdef ZENOHCXX_ZENOHC
        init_log_from_env_or("error");
#endif
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
