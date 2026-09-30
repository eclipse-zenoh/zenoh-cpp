//
// Copyright (c) 2026 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//

#pragma once

#include <deque>
#include <functional>
#include <random>
#include <string>
#include <utility>

#include "zenoh.hxx"

inline zenoh::Config test_config() {
    auto config = zenoh::Config::create_default();

#ifdef ZENOHCXX_ZENOHPICO
    // Pico's multicast scouting cannot discover the loopback-only test router, so connect directly instead.
    config.insert(Z_CONFIG_CONNECT_KEY, "tcp/127.0.0.1:7447");
    config.insert(Z_CONFIG_MULTICAST_SCOUTING_KEY, "false");
#endif

    return config;
}

using ConfigFactory = std::function<zenoh::Config()>;

inline zenoh::Config default_config() { return zenoh::Config::create_default(); }

#ifdef ZENOHCXX_ZENOHPICO
// Zenoh-pico may keep pointers to inserted config values instead of copying them, so the values must outlive both
// the config and the session opened from it. Keep dynamically built values until the test process exits.
inline const char* persistent_c_str(const std::string& value) {
    static std::deque<std::string> storage;
    return storage.emplace_back(value).c_str();
}
#endif

// Restricts the session to the given endpoints: with scouting disabled it cannot discover sessions of other
// test processes running on the same host.
inline void isolate_config(zenoh::Config& config, const std::string& listen, const std::string& connect) {
#ifdef ZENOHCXX_ZENOHC
    config.insert_json5("scouting/multicast/enabled", "false");
    config.insert_json5("scouting/gossip/enabled", "false");
    config.insert_json5("listen/endpoints", listen.empty() ? "[]" : "[\"" + listen + "\"]");
    config.insert_json5("listen/exit_on_failure", "true");
    config.insert_json5("connect/endpoints", connect.empty() ? "[]" : "[\"" + connect + "\"]");
    // Make Session::open wait until the connection is established or fail, instead of retrying in background.
    config.insert_json5("connect/timeout_ms", "5000");
    config.insert_json5("connect/exit_on_failure", "true");
#else
    // Pico sessions can only listen in peer mode.
    config.insert(Z_CONFIG_MODE_KEY, "peer");
    config.insert(Z_CONFIG_MULTICAST_SCOUTING_KEY, "false");
    if (!listen.empty()) config.insert(Z_CONFIG_LISTEN_KEY, persistent_c_str(listen));
    if (!connect.empty()) config.insert(Z_CONFIG_CONNECT_KEY, persistent_c_str(connect));
#endif
}

// Opens a session listening on a random loopback TCP port and returns it together with its locator.
// The port is not probed in advance: probing and releasing a free port would let another test process take it
// before the session binds it. Instead, bind failures are retried with another random port. The port range is
// below the ephemeral port ranges of Linux, macOS and Windows, so it rarely clashes with outgoing connections.
inline std::pair<zenoh::Session, std::string> open_listening_session(
    const ConfigFactory& make_config = default_config) {
    constexpr int max_attempts = 16;
    std::random_device random;
    std::uniform_int_distribution<unsigned> ports(20000, 32767);
    for (int attempt = 1;; ++attempt) {
        auto locator = "tcp/127.0.0.1:" + std::to_string(ports(random));
        auto config = make_config();
        isolate_config(config, locator, "");
        zenoh::ZResult result;
        auto session =
            zenoh::Session::open(std::move(config), zenoh::Session::SessionOptions::create_default(), &result);
        if (result == Z_OK) return {std::move(session), std::move(locator)};
        if (attempt == max_attempts) throw zenoh::ZException("Failed to open listening test session", result);
    }
}

// Opens a session connected only to the given locator.
inline zenoh::Session open_connecting_session(const std::string& locator, zenoh::Config config = default_config()) {
    isolate_config(config, "", locator);
    return zenoh::Session::open(std::move(config));
}

// Opens two sessions linked directly to each other over loopback and to nothing else.
inline std::pair<zenoh::Session, zenoh::Session> open_linked_session_pair(
    const ConfigFactory& make_listener_config = default_config, zenoh::Config connector_config = default_config()) {
    auto [listener, locator] = open_listening_session(make_listener_config);
    auto connector = open_connecting_session(locator, std::move(connector_config));
    return {std::move(listener), std::move(connector)};
}

// Opens two connected sessions for data exchange tests.
// On zenoh-c the sessions are linked directly, so tests running concurrently on the same host (e.g. parallel CI
// jobs using the same key expressions) cannot receive each other's data.
// On zenoh-pico both sessions connect to the test router started by run_with_router.sh.
inline std::pair<zenoh::Session, zenoh::Session> open_test_session_pair(const ConfigFactory& make_config1 = test_config,
                                                                        zenoh::Config config2 = test_config()) {
#ifdef ZENOHCXX_ZENOHC
    return open_linked_session_pair(make_config1, std::move(config2));
#else
    auto session1 = zenoh::Session::open(make_config1());
    auto session2 = zenoh::Session::open(std::move(config2));
    return {std::move(session1), std::move(session2)};
#endif
}
