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

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <string_view>

#include "../zenohc.hxx"
#include "base.hxx"
#include "interop.hxx"

namespace zenoh {
/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A representation of a Zenoh ID.
///
/// In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
class Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Construct a Zenoh ID from its 16-byte, least-significant-byte-first representation.
    /// @param bytes byte sequence in the same order as returned by bytes(). Must not be all zero.
    /// The bytes are copied and need not outlive the ID.
    /// @param err if not null, receives Z_OK on success or Z_EINVAL for an all-zero ID;
    /// otherwise failure raises ZException (or aborts when exceptions are disabled).
    /// On failure with an error pointer, the object contains zero bytes and must not be used as a valid ID.
    explicit Id(const std::array<uint8_t, 16>& bytes, ZResult* err = nullptr) : Copyable(::z_id_t{}) {
        const ZResult result =
            std::any_of(bytes.begin(), bytes.end(), [](uint8_t byte) { return byte != 0; }) ? Z_OK : Z_EINVAL;
        __ZENOH_RESULT_CHECK(result, err, "Failed to construct Id: all-zero ID");
        if (result == Z_OK) std::copy(bytes.begin(), bytes.end(), this->_0.id);
    }

    /// @name Methods

    /// Return the byte sequence of the ``Id``.
    const std::array<uint8_t, 16>& bytes() const { return *reinterpret_cast<const std::array<uint8_t, 16>*>(&_0.id); }

    /// @brief Formats the ``Id`` into 16-digit hex string (LSB-first order).
    std::string to_string() const {
        ::z_owned_string_t s;
        ::z_id_to_string(interop::as_copyable_c_ptr(*this), &s);
        std::string ss(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return ss;
    }

    /// @name Operators

    /// @brief Equality relation.
    /// @param other an id to compare with.
    /// @return ``true`` if both zenoh ids are equal, ``false`` otherwise.
    bool operator==(const Id& other) const { return this->bytes() == other.bytes(); };
};

/// @brief Print ``Id`` in the hex format.
inline std::ostream& operator<<(std::ostream& os, const Id& id) {
    ::z_owned_string_t s;
    ::z_id_to_string(interop::as_copyable_c_ptr(id), &s);
    os << std::string_view(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
    ::z_drop(::z_move(s));
    return os;
}
}  // namespace zenoh
