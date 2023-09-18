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

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "zenoh.hxx"
using namespace zenoh;

struct args_t {
    unsigned char help_requested;
    unsigned int number_of_pings;
    unsigned int size;
    unsigned int warmup_ms;
    unsigned int timeout_ms;
    char* config_path;
};
struct args_t parse_args(int argc, char** argv);

int _main(int argc, char** argv) {
    using namespace std::literals;
    std::mutex mutex;
    std::condition_variable condvar;
    auto args = parse_args(argc, argv);

    if (args.help_requested) {
        std::cout << "\
		-n (optional, int, default=100): the number of pings to be attempted\n\
		-s (optional, int, default=8): the size of the payload embedded in the ping and repeated by the pong\n\
		-w (optional, int, default=1000): the warmup time in ms during which pings will be emitted but not measured\n\
		-t (optional, int, default=100): the timeout for any individual ping, in ms.\n\
		-c (optional, string, disabled when backed by pico): the path to a configuration file for the session. If this option isn't passed, the default configuration will be used.\n\
		";
        return 1;
    }
    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (args.config_path) {
        config = expect<Config>(config_from_file(args.config_path));
    }
#endif
    std::cout << "Opening session...\n";
    auto session = expect<Session>(open(std::move(config)));

    auto sub = expect<Subscriber>(
        session.declare_subscriber("test/pong", [&condvar](const Sample&) mutable { condvar.notify_one(); }));
    auto pub = expect<Publisher>(session.declare_publisher("test/ping"));
    std::vector<char> data(args.size);
    std::unique_lock lock(mutex);
    if (args.warmup_ms) {
        auto end = std::chrono::steady_clock::now() + (1ms * args.warmup_ms);
        while (std::chrono::steady_clock::now() < end) {
            pub.put(BytesView(data.data(), data.size()));
            condvar.wait_for(lock, 1s);
        }
    }
    for (unsigned int i = 0; i < args.number_of_pings; i++) {
        auto start = std::chrono::steady_clock::now();
        pub.put(BytesView(data.data(), data.size()));
        if (condvar.wait_for(lock, 1s) == std::cv_status::timeout) {
            std::cout << "TIMEOUT seq=" << i << "\n";
            continue;
        }
        auto rtt =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
        std::cout << args.size << " bytes: seq=" << i << " rtt=" << rtt << "µs"
                  << " lat=" << rtt / 2 << "µs\n";
    }
    lock.unlock();
    return 0;
}

char* getopt(int argc, char** argv, char option) {
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (len >= 2 && argv[i][0] == '-' && argv[i][1] == option) {
            if (len > 2 && argv[i][2] == '=') {
                return argv[i] + 3;
            } else if (i + 1 < argc) {
                return argv[i + 1];
            }
        }
    }
    return NULL;
}

struct args_t parse_args(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            struct args_t args;
            args.help_requested = 1;
            return args;
        }
    }
    char* arg = getopt(argc, argv, 's');
    unsigned int size = 8;
    if (arg) {
        size = atoi(arg);
    }
    arg = getopt(argc, argv, 'n');
    unsigned int number_of_pings = 100;
    if (arg) {
        number_of_pings = atoi(arg);
    }
    arg = getopt(argc, argv, 'w');
    unsigned int warmup_ms = 1000;
    if (arg) {
        warmup_ms = atoi(arg);
    }
    arg = getopt(argc, argv, 't');
    unsigned int timeout_ms = 100;
    if (arg) {
        timeout_ms = atoi(arg);
    }
    struct args_t args;
    args.help_requested = 0;
    args.number_of_pings = number_of_pings;
    args.size = size;
    args.warmup_ms = warmup_ms;
    args.timeout_ms = timeout_ms;
    args.config_path = getopt(argc, argv, 'c');
    return args;
}

int main(int argc, char** argv) {
    try {
        return _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
