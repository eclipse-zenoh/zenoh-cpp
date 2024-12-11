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

#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"s", "selector"}, "SELECTOR", "Query selector (string)", "demo/example/**")
            .named_value({"p", "payload"}, "PAYLOAD", "Query payload (string)", "")
            .named_value({"t", "target"}, "TARGET", "Query target (BEST_MATCHING | ALL | ALL_COMPLETE)",
                         "BEST_MATCHING")
            .named_value({"o", "timeout"}, "TIMEOUT", "Timeout in ms (number)", "10000")
            .run();

    uint64_t timeout_ms = std::atoi(args.value("timeout").data());
    QueryTarget query_target = parse_query_target(args.value("target"));
    Selector selector = parse_selector(args.value("selector"));
    auto payload = args.value("payload");

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << args.value("selector") << "'...\n";

    std::mutex m;
    std::condition_variable done_signal;
    bool done = false;

    auto on_reply = [](const Reply &reply) {
        if (reply.is_ok()) {
            const auto &sample = reply.get_ok();
            std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                      << sample.get_payload().as_string() << "')\n";
        } else {
            std::cout << "Received an error :" << reply.get_err().get_payload().as_string() << "\n";
        }
    };

    auto on_done = [&m, &done, &done_signal]() {
        std::lock_guard lock(m);
        done = true;
        done_signal.notify_all();
    };

    Session::GetOptions options;
    options.target = query_target;
    if (!payload.empty()) {
        options.payload = payload;
    }
    options.timeout_ms = timeout_ms;
    session.get(selector.key_expr, selector.parameters, on_reply, on_done, std::move(options));

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
