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
