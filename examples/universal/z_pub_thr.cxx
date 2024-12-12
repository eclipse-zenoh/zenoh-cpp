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
//
#include <stdio.h>
#include <string.h>

#include <numeric>
#include <vector>

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    std::string_view keyexpr = "test/thr";
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .positional("PAYLOAD_SIZE", "Size of the payload to publish (number)")
            .named_value({"p", "priority"}, "PRIOIRTY", "Priority for sending data (number [1 - 7])", "5")
            .named_flag({"express"}, "Batch messages")
            .run();

    auto len = std::atoi(args.positional(0).data());
    auto priority = parse_priority(args.value("priority"));
    auto express = args.flag("express");

    std::vector<uint8_t> data(len);
    std::iota(data.begin(), data.end(), uint8_t{0});
    Bytes payload = std::move(data);

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on " << keyexpr << "...\n";

    Session::PublisherOptions pub_options;
    pub_options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
    pub_options.priority = priority;
    pub_options.is_express = express;
    auto pub = session.declare_publisher(KeyExpr(keyexpr), std::move(pub_options));

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(payload.clone());
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
