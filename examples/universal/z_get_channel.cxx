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

#include <iostream>
#include <string>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *value = "Get from C++";
    Config config = parse_args(argc, argv, {}, {{"key_expression", &expr}, {"payload value", &value}});

    KeyExpr keyexpr(expr);

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Sending Query '" << expr << "'...\n";
#if __cpp_designated_initializers >= 201707L
    auto replies = session.get(keyexpr, "", channels::FifoChannel(16),
                               {.target = QueryTarget::Z_QUERY_TARGET_ALL, .payload = value});
#else
    Session::GetOptions options;
    options.target = QueryTarget::Z_QUERY_TARGET_ALL;
    options.payload = value;
    auto replies = session.get(keyexpr, "", channels::FifoChannel(16), std::move(options));
#endif

    for (auto res = replies.recv(); std::holds_alternative<Reply>(res); res = replies.recv()) {
        const auto &sample = std::get<Reply>(res).get_ok();
        std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                  << sample.get_payload().as_string() << "')\n";
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
