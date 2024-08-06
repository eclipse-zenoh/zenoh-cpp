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
        __ZENOH_RESULT_CHECK(::z_encoding_from_substr(&this->_0, s.data(), s.size()), err,
                             std::string("Failed to create encoding from ").append(s));
    }

    /// @brief Copy contructor
    Encoding(const Encoding& other)
        :Encoding() {
        ::z_encoding_clone(&this->_0, other.loan());
    };

    Encoding(Encoding&& other) = default;

    /// @name Methods

    /// @brief Get string representation of encoding.
    std::string as_string() const {
        ::z_owned_string_t s;
        ::z_encoding_to_string(this->loan(), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Set a schema to this encoding from a string.
    /// 
    /// Zenoh does not define what a schema is and its semantics is left to the implementer.
    /// E.g. a common schema for `text/plain` encoding is `utf-8`.
    void set_schema(std::string_view schema, ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(
            ::z_encoding_set_schema_from_substr(this->loan(), schema.data(), schema.size()),
            err,
            "Failed to set encoding schema"
        );
    }

    /// @name Operators

    /// @brief Assignment operator.
    Encoding& operator=(const Encoding& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_encoding_clone(&this->_0, other.loan());
        }
        return *this;
    };

    Encoding& operator=(Encoding&& other) = default;
};

}  // namespace zenoh