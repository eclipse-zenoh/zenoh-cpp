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

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

void printlocators(const StrArrayView &locs) {
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

    int count = 0;
    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_hello = [&count](Hello hello) {
        printhello(hello);
        std::cout << std::endl;
        count++;
    };

    auto on_end_scouting = [&m, &done, &done_signal, &count]() {
        if (!count) std::cout << "Did not find any zenoh process.\n";
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    std::cout << "Scout starting" << std::endl;

    scout(config.create_scouting_config(), {on_hello, on_end_scouting});

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