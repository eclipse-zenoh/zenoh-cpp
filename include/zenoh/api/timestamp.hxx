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
#include "base.hxx"
#include "id.hxx"
#include "interop.hxx"

namespace zenoh {
/// Zenoh <a href=https://zenoh.io/docs/manual/abstractions/#timestamp>Timestamp</a>.
class Timestamp : public Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Get the NTP64 time part of the timestamp.
    /// @return time in NTP64 format.
    uint64_t get_time() const { return ::z_timestamp_ntp64_time(&this->inner()); }

    /// @brief Get the unique id of the timestamp.
    /// @return session id associated with this timestamp.
    Id get_id() const { return interop::into_copyable_cpp_obj<Id>(::z_timestamp_id(&this->inner())); }
};

}  // namespace zenoh