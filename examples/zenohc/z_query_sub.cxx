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

const char *kind_to_str(SampleKind kind) {
    switch (kind) {
        case SampleKind::Z_SAMPLE_KIND_PUT:
            return "PUT";
        case SampleKind::Z_SAMPLE_KIND_DELETE:
            return "DELETE";
        default:
            return "UNKNOWN";
    }
}

int _main(int argc, char **argv) {
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"k", "key"}, "KEY_EXPRESSION", "Key expression to subscriber to (string)", "demo/example/**")
            .named_value({"q", "query"}, "Query",
                         "Selector to use for queries (by default it's same as 'KEY_EXPRESSION') (string)", "")
            .run();

    auto keyexpr = args.value("key");
    auto query = args.value("query");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Querying Subscriber on '" << keyexpr << "' with initial query on '" << query << "'"
              << std::endl;
    Session::QueryingSubscriberOptions opts;

    if (!query.empty()) {
        opts.query_keyexpr = KeyExpr(query);
        opts.query_accept_replies = ReplyKeyExpr::ZC_REPLY_KEYEXPR_ANY;
    }
    auto querying_subscriber = session.declare_querying_subscriber(keyexpr, channels::FifoChannel(16), std::move(opts));

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (auto res = querying_subscriber.handler().recv(); std::holds_alternative<Sample>(res);
         res = querying_subscriber.handler().recv()) {
        const auto &sample = std::get<Sample>(res);
        std::cout << ">> [Subscriber] Received " << kind_to_str(sample.get_kind()) << " ('"
                  << sample.get_keyexpr().as_string_view() << "': '" << sample.get_payload().as_string() << "')"
                  << std::endl;
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        init_log_from_env_or("error");
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
