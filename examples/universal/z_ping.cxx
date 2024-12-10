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

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <numeric>

#include "../getargs.hxx"
#include "zenoh.hxx"

using namespace zenoh;

int _main(int argc, char** argv) {
    using namespace std::literals;
    std::mutex mutex;
    std::condition_variable condvar;

    auto&& [config, args] =
        ConfigCliArgParser(argc, argv)
            .positional("PAYLOAD_SIZE", "Size of the payload to publish (number)")
            .named_value({"n", "samples"}, "SAMPLES", "The number of pings to be attempted (number)", "100")
            .named_value({"w", "warmup"}, "WARMUP",
                         "The warmup time in ms during which pings will be emitted but not measured (number)", "1000")
            .named_flag({"no-express"}, "Disable message batching")
            .run();

    size_t payload_size = std::atoi(args.positional(0).data());
    size_t number_of_pings = std::atoi(args.value("samples").data());
    size_t warmup_ms = std::atoi(args.value("warmup").data());
    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    auto sub = session.declare_subscriber(
        KeyExpr("test/pong"), [&condvar](const Sample&) mutable { condvar.notify_one(); }, closures::none);

    Session::PublisherOptions opts;
    opts.is_express = !args.flag("no-express");
    auto pub = session.declare_publisher(KeyExpr("test/ping"), std::move(opts));
    std::vector<uint8_t> data(payload_size);
    std::iota(data.begin(), data.end(), uint8_t{0});
    Bytes payload = std::move(data);

    std::unique_lock lock(mutex);
    if (warmup_ms > 0) {
        auto end = std::chrono::steady_clock::now() + (1ms * warmup_ms);
        while (std::chrono::steady_clock::now() < end) {
            pub.put(payload.clone());
            condvar.wait_for(lock, 1s);
        }
    }
    for (unsigned int i = 0; i < number_of_pings; i++) {
        auto start = std::chrono::steady_clock::now();
        pub.put(payload.clone());
        if (condvar.wait_for(lock, 1s) == std::cv_status::timeout) {
            std::cout << "TIMEOUT seq=" << i << "\n";
            continue;
        }
        auto rtt =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
        std::cout << payload_size << " bytes: seq=" << i << " rtt=" << rtt << "µs" << " lat=" << rtt / 2 << "µs\n";
    }
    lock.unlock();
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
