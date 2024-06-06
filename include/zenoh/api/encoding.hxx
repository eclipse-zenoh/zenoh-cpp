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

#include <string>

namespace zenoh {
class Encoding : public Owned<::z_owned_encoding_t> {
public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Default encoding
    Encoding() : Owned(nullptr) {}

    /// @brief Constructs encoding from string
    Encoding(std::string_view s, ZError* err = nullptr) : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(
            ::z_encoding_from_substring(&this->_0, s.data(), s.size()),
            err,
            std::string("Failed to create encoding from ").append(s)
        );
    }

    /// @brief Converts encoding to a string
    std::string as_string() const {
        ::z_owned_string_t s;
        ::z_encoding_to_string(this->loan(), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }
};

}