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

#include <chrono>
#include <iostream>
#include <thread>

#include "../getargs.h"
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

void data_handler(const Sample &sample) {
// if Zenoh is built without SHM support, the only buffer type it can receive is RAW
#if !defined(Z_FEATURE_SHARED_MEMORY)
    const char *payload_type = "RAW";
#endif

// if Zenoh is built with SHM support but without SHM API (that is unstable), it can
// receive buffers of any type, but there is no way to detect the buffer type
#if defined(Z_FEATURE_SHARED_MEMORY) && !defined(Z_FEATURE_UNSTABLE_API)
    const char *payload_type = "UNKNOWN";
#endif

// if Zenoh is built with SHM support and with SHM API, we can detect the exact buffer type
#if defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
    const char *payload_type = "RAW";
    {
        ZResult result;
        sample.get_payload().deserialize<ZShm>(&result);
        if (result == Z_OK) {
            payload_type = "SHM";
        }
    }
#endif

    std::cout << ">> [Subscriber] Received [" << payload_type << "] " << kind_to_str(sample.get_kind()) << " ('"
              << sample.get_keyexpr().as_string_view() << "' : '" << sample.get_payload().deserialize<std::string>()
              << "')\n";
}

int _main(int argc, char **argv) {
    const char *expr = "demo/example/**";
    const char *locator = nullptr;
    const char *config_file = nullptr;

    getargs(argc, argv, {}, {{"key expression", &expr}, {"locator", &locator}},
            {{"-c", {"config file", &config_file}}});

    Config config = Config::create_default();
    if (config_file) {
        config = Config::from_file(config_file);
    }

    ZResult err;
    if (locator) {
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        config.insert_json5(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str(), &err);

        if (err != Z_OK) {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    KeyExpr keyexpr(expr);

    std::cout << "Opening session..." << std::endl;
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Subscriber on '" << keyexpr.as_string_view() << "'..." << std::endl;
    auto subscriber = session.declare_subscriber(keyexpr, &data_handler, closures::none);
    std::cout << "Subscriber on '" << subscriber.get_keyexpr().as_string_view() << "' declared" << std::endl;

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