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
    const char *value = "";
    const char *config_file = nullptr;
    getargs(argc, argv, {}, {{"key expression", &expr}, {"value", &value}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &config_file}}}
#endif
    );

    Config config = Config::create_default();
#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = Config::from_file(config_file);
    }
#endif

    KeyExpr keyexpr(expr);

    printf("Opening session...\n");
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << expr << "'...\n";

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_reply = [](const Reply &reply) {
        if (reply.is_ok()) {
            const Sample &sample = reply.get_ok();
            std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                      << sample.get_payload().as_string() << "')\n";
#if defined(Z_FEATURE_UNSTABLE_API)
            auto attachment = sample.get_attachment();
            if (!attachment.has_value()) return;
            // we expect attachment in the form of key-value pairs
            auto attachment_deserialized =
                ext::deserialize<std::unordered_map<std::string, std::string>>(attachment->get());
            for (auto &&[key, value] : attachment_deserialized) {
                std::cout << "   attachment: " << key << ": '" << value << "'\n";
            }
#endif
        } else {
            std::cout << "Received an error :" << reply.get_err().get_payload().as_string() << "\n";
        }
    };

    auto on_done = [&m, &done, &done_signal]() {
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    std::unordered_map<std::string, std::string> attachment = {{"Source", "C++"}};

    Session::GetOptions options;
    options.target = QueryTarget::Z_QUERY_TARGET_ALL;
    options.payload = value;
#if defined(Z_FEATURE_UNSTABLE_API)
    options.attachment = ext::serialize(attachment);
#endif
    session.get(keyexpr, "", on_reply, on_done, std::move(options));

    std::unique_lock lock(m);
    done_signal.wait(lock, [&done] { return done; });

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
