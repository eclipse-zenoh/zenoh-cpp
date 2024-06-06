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
#include "base.hxx"
#include "../zenohc.hxx"
#include "id.hxx"

namespace zenoh {
/// Zenoh <a href=https://zenoh.io/docs/manual/abstractions/#timestamp>Timestamp</a>.
struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc

    /// @name Methods

    /// @brief Get the NPT64 time part of timestamp
    /// @return time in NPT64 format.
    uint64_t get_time() const { return ::z_timestamp_npt64_time(&this->inner()); }

    /// @brief Get the unique id of the timestamp
    /// @return unique id
    Id get_id() const { return ::z_timestamp_id(&this->inner()); }
};

}