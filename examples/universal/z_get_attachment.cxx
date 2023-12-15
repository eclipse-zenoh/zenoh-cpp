//
// Copyright (c) 2023 ZettaScale Technology
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
#include <map>
#include <mutex>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *value = nullptr;
    const char *configfile = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}}
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

    KeyExprView keyexpr(expr);
    if (!keyexpr.check()) {
        printf("%s is not a valid key expression", expr);
        exit(-1);
    }

    printf("Opening session...\n");
    auto session = expect<Session>(open(std::move(config)));

    std::cout << "Sending Query '" << expr << "'...\n";
    GetOptions opts;
    opts.set_target(Z_QUERY_TARGET_ALL);
    opts.set_value(value);
#ifdef ZENOHCXX_ZENOHC
    // allocate attachment map
    std::map<std::string, std::string> amap;
    // set it as an attachment
    opts.set_attachment(amap);
    // add some value
    amap.insert(std::pair("source", "C++"));
#endif

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_reply = [](Reply &&reply) {
        auto result = reply.get();

        if (auto sample = std::get_if<Sample>(&result)) {
            std::cout << "Received ('" << sample->get_keyexpr().as_string_view() << "' : '"
                      << sample->get_payload().as_string_view() << "')\n";
#ifdef ZENOHCXX_ZENOHC
            if (sample->get_attachment().check()) {
                // reads full attachment
                sample->get_attachment().iterate([](const BytesView &key, const BytesView &value) -> bool {
                    std::cout << "   attachment: " << key.as_string_view() << ": '" << value.as_string_view() << "'\n";
                    return true;
                });

                // reads particular attachment item
                auto index = sample->get_attachment().get("source");
                if (index != "") {
                    std::cout << "   event source: " << index.as_string_view() << std::endl;
                }
            }
#endif
        } else if (auto error = std::get_if<ErrorMessage>(&result)) {
            std::cout << "Received an error :" << error->as_string_view() << "\n";
        }
    };

    auto on_done = [&m, &done, &done_signal]() {
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    session.get(keyexpr, "", {on_reply, on_done}, opts);

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
