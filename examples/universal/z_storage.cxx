//
// Copyright (c) 2025 ZettaScale Technology
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
            .named_value({"k", "key"}, "KEY_EXPRESSION", "The selection of resources to store", "demo/example/**")
            .named_flag({"complete"}, "Flag to indicate whether the storage is complete w.r.t. the key expression")
            .run();
    KeyExpr keyexpr(args.value("key"));

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));
    std::unordered_map<std::string_view, Sample> storage;
    std::mutex storage_mutex;

    auto sub_handler = [&storage, &storage_mutex](Sample &sample) {
        std::lock_guard<std::mutex> lock(storage_mutex);
        std::cout << ">> [Subscriber] Received " << kind_to_str(sample.get_kind()) << " ('"
                  << sample.get_keyexpr().as_string_view() << "' : '" + sample.get_payload().as_string() << "')\n";

        switch (sample.get_kind()) {
            case SampleKind::Z_SAMPLE_KIND_PUT:
                // Note: it is safe to use string_view as a key, since it references internal sample data,
                // so it is guaranteed to stay valid as long as corresponding map value exists.
                storage.erase(sample.get_keyexpr().as_string_view());
                storage.insert({sample.get_keyexpr().as_string_view(), std::move(sample)});
                break;
            case SampleKind::Z_SAMPLE_KIND_DELETE:
                storage.erase(sample.get_keyexpr().as_string_view());
                break;
        }
    };

    std::cout << "Declaring Subscriber on '" << keyexpr.as_string_view() << "'..." << std::endl;
    auto subscriber = session.declare_subscriber(keyexpr, sub_handler, closures::none);

    auto qbl_handler = [&storage, &storage_mutex](Query &query) {
        std::lock_guard<std::mutex> lock(storage_mutex);
        std::cout << ">> [Queryable ] Received Query '" << query.get_keyexpr().as_string_view() << "?"
                  << query.get_parameters() << "'\n";
        for (const auto &[k, v] : storage) {
            if (query.get_keyexpr().intersects(v.get_keyexpr())) {
                query.reply(v.get_keyexpr(), v.get_payload().clone());
            }
        }
    };

    std::cout << "Declaring Queryable on '" << keyexpr.as_string_view() << "'..." << std::endl;
    Session::QueryableOptions opts;
    opts.complete = args.flag("complete");
    auto queryable = session.declare_queryable(keyexpr, qbl_handler, closures::none, std::move(opts));

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
