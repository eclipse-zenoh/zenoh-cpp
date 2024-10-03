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

#include <iostream>

#include "../getargs.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char** argv) {
    const char* value = "Get from C++";
    Config config = parse_args(argc, argv, {});

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    std::cout << "own id: " << session.get_zid() << std::endl;

    std::cout << "routers ids:\n";
    for (const auto zid : session.get_routers_z_id()) {
        std::cout << zid << "\n";
    }

    std::cout << "peers ids:\n";
    for (const auto zid : session.get_peers_z_id()) {
        std::cout << zid << "\n";
    }
#endif
    return 0;
}

int main(int argc, char** argv) {
    try {
#ifdef ZENOHCXX_ZENOHC
        init_log_from_env_or("error");
#endif
        return _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
