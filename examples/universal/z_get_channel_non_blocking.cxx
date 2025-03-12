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

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;
using namespace std::chrono_literals;

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

    Session::GetOptions options;
    options.target = query_target;
    if (!payload.empty()) {
        options.payload = payload;
    }
    options.timeout_ms = timeout_ms;
    auto replies = session.get(selector.key_expr, selector.parameters, channels::FifoChannel(16), std::move(options));

    while (true) {
        auto res = replies.try_recv();
        if (std::holds_alternative<channels::RecvError>(res)) {
            if (std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA) {
                std::cout << ".";
                std::this_thread::sleep_for(1s);
                continue;
            } else {  // channel is closed - no more replies will be received
                break;
            }
        }
        const auto &sample = std::get<Reply>(res).get_ok();
        std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                  << sample.get_payload().as_string() << "')\n";
    }
    std::cout << std::endl;

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
