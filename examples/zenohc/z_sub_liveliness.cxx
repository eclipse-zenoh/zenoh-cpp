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

#include <chrono>
#include <iostream>
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

void data_handler(const Sample &sample) {
    if (sample.get_kind() == Z_SAMPLE_KIND_PUT) {
        std::cout << ">> [LivelinessSubscriber] New alive token ('" << sample.get_keyexpr().as_string_view() << "')\n";
    } else if (sample.get_kind() == Z_SAMPLE_KIND_DELETE) {
        std::cout << ">> [LivelinessSubscriber] Dropped token ('" << sample.get_keyexpr().as_string_view() << "')\n";
    }
}

int _main(int argc, char **argv) {
    const char *expr = "group1/**";
    Config config = parse_args(argc, argv, {}, {{"key_expression", &expr}});

    KeyExpr keyexpr(expr);

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Liveliness Subscriber on '" << keyexpr.as_string_view() << "'..." << std::endl;
    auto subscriber = session.liveliness_declare_subscriber(keyexpr, &data_handler, closures::none);

    std::cout << "Press CTRL-C to quit...\n";
    while (true) {
        std::this_thread::sleep_for(1s);
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