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
/// @brief Enable logging.
///
/// User may set environment variable RUST_LOG to values *debug* | *info* | *warn* | *error* to show diagnostic output.
///
/// @note zenoh-c only
inline void init_logging() { ::zc_init_logging(); }
#endif

}