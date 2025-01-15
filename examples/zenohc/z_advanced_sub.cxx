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
#include <chrono>
#include <iostream>
#include <limits>
#include <sstream>
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
            .named_value({"k", "key"}, "KEY_EXPRESSION", "Key expression to subscriber to (string)", "demo/example/**")
            .run();

    auto keyexpr = args.value("key");

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    ext::SessionExt::AdvancedSubscriberOptions opts;
    opts.history.emplace().detect_late_publishers = true;
    opts.history->detect_late_publishers = true;
    opts.recovery.emplace();  // enable recovery based on received heartbeats from ext::AdvancedPublisher
    // alternatively recovery can be triggered based on missed sample detection via periodic queries:
    // opts.recovery.emplace().periodic_queries_period_ms = 1000;
    opts.subscriber_detection = true;

    auto data_handler = [](const Sample &sample) {
        std::cout << ">> [Subscriber] Received " << kind_to_str(sample.get_kind()) << " ('"
                  << sample.get_keyexpr().as_string_view() << "' : '" << sample.get_payload().as_string() << "')";
        std::cout << std::endl;
    };

    auto missed_sample_handler = [](const ext::Miss &miss) {
        std::cout << ">> [Subscriber] Missed " << miss.nb << " samples from '" << miss.source.id() << "' !!!"
                  << std::endl;
    };

    std::cout << "Declaring AdvancedSubscriber on '" << keyexpr << "'" << std::endl;
    auto advanced_subscriber =
        session.ext().declare_advanced_subscriber(keyexpr, data_handler, closures::none, std::move(opts));

    advanced_subscriber.declare_background_sample_miss_listener(missed_sample_handler, closures::none);

    std::cout << "Press CTRL-C to quit..." << std::endl;
    while (true) {
        std::this_thread::sleep_for(1s);
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
