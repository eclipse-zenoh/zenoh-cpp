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
#include "../detail/interop.hxx"
#include "id.hxx"

namespace zenoh {
/// Zenoh <a href=https://zenoh.io/docs/manual/abstractions/#timestamp>Timestamp</a>.
class Timestamp :public Copyable<::z_timestamp_t> {
public:
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc

    /// @name Constructors

    /// @brief Create Timestamp from id and NTP64 time.
    /// @param id unique id associated with timestamp (normally provided by Zenoh router).
    /// @param ntp64_time time in NTP64 format.
    Timestamp(const Id& id, uint64_t ntp64_time)
        :Copyable({}) {
        z_timestamp_new(&this->inner(), detail::as_copyable_c_ptr(id), ntp64_time);
    }
    
    /// @name Methods

    /// @brief Get the NTP64 time part of the timestamp.
    /// @return time in NTP64 format.
    uint64_t get_time() const { return ::z_timestamp_npt64_time(&this->inner()); }

    /// @brief Get the unique id of the timestamp.
    /// @return Id associated with this timestamp.
    Id get_id() const { return ::z_timestamp_id(&this->inner()); }
};

}