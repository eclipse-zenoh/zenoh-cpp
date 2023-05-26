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

int _main(int argc, char **argv) {
    Config config;

    std::cout << "Opening session...\n";
    auto session = std::get<Session>(open(std::move(config)));

    auto pub = std::get<Publisher>(session.declare_publisher("test/pong"));
    auto sub = std::get<Subscriber>(
        session.declare_subscriber("test/ping", std::move([pub = std::move(pub)](const Sample *sample) mutable {
                                       if (sample) {
                                           pub.put(sample->get_payload());
                                       }
                                   })));
    std::cout << "Pong ready, press any key to quit\n";
    std::getchar();
    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
