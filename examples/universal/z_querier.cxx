//
// Copyright (c) 2024 ZettaScale Technology
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
#include <string.h>

#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
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
#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
            .named_flag({"add-matching-listener"}, "Add matching listener")
#endif
            .run();

    uint64_t timeout_ms = std::atoi(args.value("timeout").data());
    QueryTarget query_target = parse_query_target(args.value("target"));
    Selector selector = parse_selector(args.value("selector"));
    auto payload = args.value("payload");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Querier on '" << selector.key_expr << "'..." << std::endl;
    Session::QuerierOptions options;
    options.target = query_target;
    options.timeout_ms = timeout_ms;
    auto querier = session.declare_querier(selector.key_expr, std::move(options));

#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
    if (args.flag("add-matching-listener")) {
        querier.declare_background_matching_listener(
            [](const MatchingStatus &s) {
                if (s.matching) {
                    std::cout << "Querier has matching queryables." << std::endl;
                } else {
                    std::cout << "Querier has NO MORE matching queryables." << std::endl;
                }
            },
            closures::none);
    }
#endif

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] ";
        if (!payload.empty()) {
            ss << payload;
        }
        auto s = ss.str();
        std::cout << "Querying '" << args.value("selector") << "' with payload '" << s << "'...\n";

        Querier::GetOptions get_options;
        get_options.payload = std::move(s);
        auto replies = querier.get(selector.parameters, channels::FifoChannel(16), std::move(get_options));
        for (auto res = replies.recv(); std::holds_alternative<Reply>(res); res = replies.recv()) {
            const auto &reply = std::get<Reply>(res);
            if (reply.is_ok()) {
                const auto &sample = std::get<Reply>(res).get_ok();
                std::cout << "Received ('" << reply.get_ok().get_keyexpr().as_string_view() << "' : '"
                          << reply.get_ok().get_payload().as_string() << "')\n";
            } else {
                std::cout << "Received an error\n";
            }
        }
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
