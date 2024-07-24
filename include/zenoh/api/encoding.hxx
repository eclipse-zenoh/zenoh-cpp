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
#include <string>

#include "../zenohc.hxx"
#include "base.hxx"

namespace zenoh {

/// @brief The <a href="https://zenoh.io/docs/manual/abstractions/#encoding"> encoding </a> of Zenoh data.
class Encoding : public Owned<::z_owned_encoding_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Construct default encoding.
    Encoding() : Owned(nullptr) {}

    /// @brief Construct encoding from string.
    Encoding(std::string_view s, ZResult* err = nullptr) : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(::z_encoding_from_substr(&this->_0, s.data(), s.size()), err,
                            std::string("Failed to create encoding from ").append(s));
    }

    /// @name Methods

    /// @brief Get string representation of encoding.
    std::string as_string() const {
        ::z_owned_string_t s;
        ::z_encoding_to_string(this->loan(), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }
};

}  // namespace zenoh