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

#include <condition_variable>
#include <iostream>
#include <mutex>

#include <thread>
#include <chrono>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;
using namespace std::chrono_literals;

void printlocators(const std::vector<std::string_view> &locs) {
    std::cout << "[";
    for (size_t i = 0; i < locs.size(); i++) {
        std::cout << "\"" << locs[i] << "\"";
        if (i < locs.size() - 1) std::cout << ", ";
    }
    std::cout << "]";
}

void printhello(const Hello &hello) {
    std::cout << "Hello { pid: " <<  hello.get_id();
    std::cout << ", whatami: " << hello.get_whatami();
    std::cout << ", locators: ";
    printlocators(hello.get_locators());
    std::cout << " }";
}

int _main(int argc, char **argv) {
    const char *locator = nullptr;
    const char *config_file = nullptr;

    getargs(argc, argv, {}, {{"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
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

    size_t count = 0;
    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_hello = [&count](const Hello& hello) {
        printhello(hello);
        std::cout << std::endl;
        count++;
    };

    auto on_end_scouting = [&m, &done, &done_signal, &count]() {
        if (count == 0) std::cout << "Did not find any zenoh process.\n";
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    std::cout << "Scout starting" << std::endl;

    scout(std::move(config), on_hello, on_end_scouting);

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

    std::cout << "Scout finished" << std::endl;

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}