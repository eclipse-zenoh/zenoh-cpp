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

#pragma once
#include "../zenohc.hxx"

namespace zenoh {

#ifdef ZENOHCXX_ZENOHC
/// @brief Initializes the zenoh runtime logger, using rust environment settings.
/// E.g.: `RUST_LOG=info` will enable logging at info level. Similarly, you can set the variable to `error` or `debug`.
/// Note that if the environment variable is not set, then logging will not be enabled.
/// See https://docs.rs/env_logger/latest/env_logger/index.html for accepted filter format.
/// @note Zenoh-c only.
inline void try_init_log_from_env() { ::zc_try_init_log_from_env(); }

/// @brief Initializes the zenoh runtime logger, using rust environment settings or the provided fallback filter.
/// E.g.: `RUST_LOG=info` will enable logging at info level. Similarly, you can set the variable to `error` or `debug`.
///
/// Note that if the environment variable is not set, then fallback filter will be used instead.
/// See https://docs.rs/env_logger/latest/env_logger/index.html for accepted filter format.
///
/// @param fallback_filter: The fallback filter if the `RUST_LOG` environment variable is not set.
/// @note Zenoh-c only.
inline void init_log_from_env_or(const std::string& fallback_filter) {
    ::zc_init_log_from_env_or(fallback_filter.c_str());
}
#endif

}  // namespace zenoh