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

#include <array>
#include <iomanip>
#include <iostream>
#include <string_view>

#include "../zenohc.hxx"
#include "base.hxx"
#include "interop.hxx"

namespace zenoh {
/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A representation a Zenoh ID.
///
/// In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
class Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// Check if the ID is valid.
    /// @return ``true`` if the ID is valid.
    bool is_valid() const { return _0.id[0] != 0; }

    /// Return the byte sequence of the ID.
    const std::array<uint8_t, 16>& bytes() const { return *reinterpret_cast<const std::array<uint8_t, 16>*>(&_0.id); }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Print ``Id`` in the hex format.
inline std::ostream& operator<<(std::ostream& os, const Id& id) {
    ::z_owned_string_t s;
    ::z_id_to_string(interop::as_copyable_c_ptr(id), &s);
    os << std::string_view(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
    ::z_drop(::z_move(s));
    return os;
}
}  // namespace zenoh