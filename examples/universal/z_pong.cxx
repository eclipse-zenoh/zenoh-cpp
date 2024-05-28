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

#include <cstdio>
#include <iostream>

#include "zenoh.hxx"
using namespace zenoh;

int _main(int, char **) {
    Config config;

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    auto pub = session.declare_publisher(KeyExpr("test/pong"));
    auto sub = session.declare_subscriber(
        KeyExpr("test/ping"), [pub = std::move(pub)](const Sample &sample) mutable { pub.put(sample.get_payload().clone()); });
    std::cout << "Pong ready, press any key to quit\n";
    std::getchar();
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
