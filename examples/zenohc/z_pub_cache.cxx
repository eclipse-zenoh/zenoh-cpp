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

#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

#include "../getargs.h"
#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

const char *default_value = "Pub from C++ zenoh-c!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-pub";
const char *default_history = "1";
const char *default_prefix = "";

int _main(int argc, char **argv) {
    const char *keyexpr = default_keyexpr;
    const char *value = default_value;
    const char *history = default_history;
    const char *prefix = default_prefix;
    Config config = parse_args(argc, argv, {}, {{"key_expression", &keyexpr}, {"payload_value", &value}},
                               {{"-i", {"history", &history}}, {"-x", {"query prefix", &prefix}}});
    config.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publication cache on '" << keyexpr << "'..." << std::endl;
    Session::PublicationCacheOptions opts;
    opts.history = std::atoi(history);
    if (!std::string(prefix).empty()) {
        opts.queryable_prefix = KeyExpr(prefix);
    }
    auto pub_cache = session.declare_publication_cache(keyexpr, std::move(opts));

    std::cout << "Publication cache on '" << keyexpr << "' declared" << std::endl;

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
        Session::PutOptions put_options;
        put_options.encoding = Encoding("text/plain");
        session.put(keyexpr, std::move(s), std::move(put_options));
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
