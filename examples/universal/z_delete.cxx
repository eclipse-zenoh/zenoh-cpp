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
#include <iostream>

#ifdef ZENOHCXX_ZENOHC
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-put";
#elif ZENOHCXX_ZENOHPICO
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-pico-put";
#endif

#include "../getargs.hxx"
#include "stdio.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"k", "key"}, "KEY_EXPRESSION", "The key expression to write to", default_keyexpr)
            .run();

    std::string_view keyexpr = args.value("key");

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Deleting resources matching '" << keyexpr << "'...\n";
    session.delete_resource(KeyExpr(keyexpr));
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