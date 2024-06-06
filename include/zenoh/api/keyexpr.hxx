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

#include <string_view>

namespace zenoh {

/// @brief Owned key expression.
///
/// See details about key expression syntax in the <a
/// href="https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md"> Key Expressions RFC</a>.
class KeyExpr : public Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a new instance from a string
    ///
    /// @param key_expr String representing key expression
    /// @param autocanonize If true the key_expr will be autocanonized, prior to constructing key expression
    /// @param err If not null the error code will be written to this location, otherwise exception will be thrown in case of error.
    explicit KeyExpr(std::string_view key_expr, bool autocanonize = true, ZError* err = nullptr) 
        : Owned(nullptr) {
        if (autocanonize) {
            size_t s = key_expr.size();
            __ZENOH_ERROR_CHECK(
                ::z_keyexpr_from_substring_autocanonize(&this->_0, key_expr.data(), &s), 
                err, 
                std::string("Failed to construct KeyExpr from: ").append(key_expr)
            );
        } else {
            __ZENOH_ERROR_CHECK(
                ::z_keyexpr_from_substring(&this->_0, key_expr.data(), key_expr.size()),
                err,
                std::string("Failed to construct KeyExpr from: ").append(key_expr)
            );
        }
    }

    /// @name Methods
    /// @brief Get underlying key expression string
    std::string_view as_string_view() const {
        ::z_view_string_t s;
        ::z_keyexpr_as_view_string(this->loan(), &s);
        return std::string_view(reinterpret_cast<const char*>(::z_string_data(::z_loan(s))), ::z_string_len(::z_loan(s)));
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param other the ``std::string_view`` to compare with
    /// @return true if the key expression is equal to the string
    bool operator==(std::string_view other) {
        if (!(*this)) return false; 
        return as_string_view() == other; 
    }

    /// @brief InEquality operator
    /// @param other the ``std::string_view`` to compare with
    /// @return false if the key expression is equal to the string
    bool operator!=(std::string_view other) {
        return !((*this) == other);
    }

    /// @brief Equality operator
    /// @param other the ``KeyExpr`` to compare with
    /// @return true if both key expressions are equal
    bool operator==(const KeyExpr& other) { return ::z_keyexpr_equals(this->loan(), other.loan()); }

    /// @brief Inequality operator
    /// @param other the ``KeyExpr`` to compare with
    /// @return false if both key expressions are equal
    bool operator!=(const KeyExpr& other) { return !(*this == other); }

    /// @brief Checks if a given ``KeyExpr`` includes the other.
    /// @param other the ``KeyExpr`` to compare with
    /// @return true if other is included in this.
    bool includes(const KeyExpr& other) {
        return ::z_keyexpr_includes(this->loan(), other.loan());
    }

    /// @brief Constructs new key expression by concatenation this with a string.
    KeyExpr concat(std::string_view s, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_keyexpr_concat(&k._0, this->loan(), s.data(), s.size()),
            err,
            std::string("Failed to concatenate KeyExpr: ").append(this->as_string_view()).append(" with ").append(s)
        );
        return k;
    }

    /// @brief Constructs new key expression by joining this with another one
    KeyExpr join(const KeyExpr& other, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_keyexpr_join(&k._0, this->loan(), other.loan()),
            err,
            std::string("Failed to join KeyExpr: ").append(this->as_string_view()).append(" with ").append(other.as_string_view())
        );
        return k;
    }

    /// @brief Checks if 2 key expressions intersect
    /// @return true if there is at least one non-empty key that is contained in both key expressions
    bool intersects(const KeyExpr& other) const { return ::z_keyexpr_intersects(this->loan(), other.loan()); }

    typedef ::z_keyexpr_intersection_level_t IntersectionLevel;

    IntersectionLevel relation_to(const KeyExpr& other) { return ::z_keyexpr_relation_to(this->loan(), other.loan()); }

    /// @brief Verifies if the string is a canonical key expression
    static bool is_canon(std::string_view s) {
        return ::z_keyexpr_is_canon(s.data(), s.size()) == Z_OK;
    }
};

}