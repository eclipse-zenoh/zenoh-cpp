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

#include <array>
#include <iostream>
#include <iomanip>

namespace zenoh {
    /// @brief A representation a Zenoh ID.
///
/// In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;

    /// @name Methods

    /// Checks if the ID is valid
    /// @return true if the ID is valid
    bool is_valid() const { return _0.id[0] != 0; }

    /// Returns the byte sequence of the ID
    const std::array<uint8_t, 16>& bytes() const { return *reinterpret_cast<const std::array<uint8_t, 16>*>(&_0.id); }
};

inline std::ostream& operator<<(std::ostream& os, const Id& id) {
    auto id_ptr = reinterpret_cast<const::z_id_t*>(&id)->id;
    for (size_t i = 0; id_ptr[i] != 0 && i < 16; i++)
        os << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(id_ptr[i]);
    return os;
}
}