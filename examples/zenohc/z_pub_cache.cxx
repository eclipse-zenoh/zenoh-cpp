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

const char *default_value = "Pub from C++ zenoh-c!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-pub";
const char *default_prefix = "";

int _main(int argc, char **argv) {
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .named_value({"k", "key"}, "KEY_EXPRESSION", "Key expression to write to (string)", default_keyexpr)
            .named_value({"v", "value"}, "VALUE", "Value to publish (string)", default_value)
            .named_value({"i", "history"}, "HISTORY", "Number of publications to keep in cache (number)", "1")
            .named_flag({"o", "complete"},
                        "Set `complete` option to true. This means that this queryable is ultimate data source, no "
                        "need to scan other queryables")
            .named_value({"x", "prefix"}, "PREFIX", "Queryable prefix", "")
            .run();

    auto keyexpr = args.value("key");
    auto value = args.value("value");
    auto history = std::atoi(args.value("history").data());
    auto complete = args.flag("complete");
    auto prefix = args.value("prefix");

    config.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publication cache on '" << keyexpr << "'..." << std::endl;
    Session::PublicationCacheOptions opts;
    opts.history = history;
    opts.queryable_complete = complete;
    if (!prefix.empty()) {
        opts.queryable_prefix = KeyExpr(prefix);
    }
    if (!std::string(prefix).empty()) {
        opts.queryable_prefix = KeyExpr(prefix);
    }
    auto pub_cache = session.declare_publication_cache(keyexpr, std::move(opts));

    std::cout << "Press CTRL-C to quit..." << std::endl;
    for (int idx = 0; idx < std::numeric_limits<int>::max(); ++idx) {
        std::this_thread::sleep_for(1s);
        std::ostringstream ss;
        ss << "[" << idx << "] " << value;
        auto s = ss.str();
        std::cout << "Putting Data ('" << keyexpr << "': '" << s << "')...\n";
        session.put(keyexpr, std::move(s));
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
