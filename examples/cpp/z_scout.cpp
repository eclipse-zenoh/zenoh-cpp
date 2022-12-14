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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "zenohcpp.h"

using namespace zenoh;

void printlocators(const StrArray &locs) {
    std::cout << "[";
    for (unsigned int i = 0; i < locs.get_len(); i++) {
        std::cout << "\"" << locs[i] << "\"";
        if (i < locs.get_len() - 1) std::cout << ", ";
    }
    std::cout << "]";
}

void printhello(const HelloView &hello) {
    std::cout << "Hello { pid: ";
    if (hello.get_id().is_some())
        std::cout << "Some(" << hello.get_id() << ")";
    else
        std::cout << "None";
    std::cout << ", whatami: ";
    if (auto s = as_cstr(hello.get_whatami()))
        std::cout << s;
    else
        std::cout << "Unknown(" << hello.whatami << ")";
    std::cout << ", locators: ";
    printlocators(hello.get_locators());
    std::cout << " }";
}

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

    int count = 0;
    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    scout(std::move(config.create_scouting_config()), [&m, &done, &done_signal, &count](Hello hello) {
        if (hello.check()) {
            printhello(hello);
            std::cout << std::endl;
            count++;
        } else {
            std::cout << "Dropping scout\n";
            if (!count) std::cout << "Did not find any zenoh process.\n";
            std::lock_guard lock(m);
            done = true;
            done_signal.notify_all();
        }
    });

    std::cout << "Scout started" << std::endl;

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

    std::cout << "Scout finished" << std::endl;

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}