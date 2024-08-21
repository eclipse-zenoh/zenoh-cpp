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
#include <string_view>

#include "../zenohc.hxx"
#include "base.hxx"

namespace zenoh {
class Session;

/// @brief A Zenoh <a href="https://zenoh.io/docs/manual/abstractions/#key-expression"> key expression </a>.
///
/// Key expression can be registered in the `zenoh::Session` object with `zenoh::Session::declare_keyexpr()` method.
/// The unique id is internally assinged to the key expression string in this case. This allows to reduce bandwith usage
/// when transporting key expressions.

class KeyExpr : public Owned<::z_owned_keyexpr_t> {
    friend Session;
    KeyExpr() : Owned(nullptr) {};

   public:
    /// @name Constructors

    /// @brief Create a new instance from a string.
    ///
    /// @param key_expr String representing key expression.
    /// @param autocanonize If true the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    explicit KeyExpr(std::string_view key_expr, bool autocanonize = true, ZResult* err = nullptr) : Owned(nullptr) {
        if (autocanonize) {
            size_t s = key_expr.size();
            __ZENOH_RESULT_CHECK(::z_keyexpr_from_substr_autocanonize(&this->_0, key_expr.data(), &s), err,
                                 std::string("Failed to construct KeyExpr from: ").append(key_expr));
        } else {
            __ZENOH_RESULT_CHECK(::z_keyexpr_from_substr(&this->_0, key_expr.data(), key_expr.size()), err,
                                 std::string("Failed to construct KeyExpr from: ").append(key_expr));
        }
    }

    /// @brief Create a new instance from a string.
    ///
    /// @param key_expr String representing key expression.
    /// @param autocanonize If true the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr(const std::string& key_expr, bool autocanonize = true, ZResult* err = nullptr)
        : KeyExpr(static_cast<std::string_view>(key_expr), autocanonize, err) {};

    /// @brief Create a new instance from a null-terminated string.
    ///
    /// @param key_expr Null-terminated string representing key expression.
    /// @param autocanonize If true the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr(const char* key_expr, bool autocanonize = true, ZResult* err = nullptr)
        : KeyExpr(std::string_view(key_expr), autocanonize, err) {};

    /// @name Methods
    /// @brief Get underlying key expression string.
    std::string_view as_string_view() const {
        ::z_view_string_t s;
        ::z_keyexpr_as_view_string(this->loan(), &s);
        return std::string_view(reinterpret_cast<const char*>(::z_string_data(::z_loan(s))),
                                ::z_string_len(::z_loan(s)));
    }

    /// @brief Check if a given ``KeyExpr`` includes the other.
    /// @param other the ``KeyExpr`` to compare with
    /// @return ``true`` if current key expression includes ``other``, i.e. contains every key belonging to the
    /// ``other``.
    bool includes(const KeyExpr& other) { return ::z_keyexpr_includes(this->loan(), other.loan()); }

    /// @brief Construct new key expression by concatenating the current one with a string.
    /// @param s A string to concatenate with the key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return A new key expression.
    KeyExpr concat(std::string_view s, ZResult* err = nullptr) const {
        KeyExpr k;
        __ZENOH_RESULT_CHECK(
            ::z_keyexpr_concat(&k._0, this->loan(), s.data(), s.size()), err,
            std::string("Failed to concatenate KeyExpr: ").append(this->as_string_view()).append(" with ").append(s));
        return k;
    }

    /// @brief Construct new key expression by joining it with another one.
    /// @param other the ``KeyExpr`` to append.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr join(const KeyExpr& other, ZResult* err = nullptr) const {
        KeyExpr k;
        __ZENOH_RESULT_CHECK(::z_keyexpr_join(&k._0, this->loan(), other.loan()), err,
                             std::string("Failed to join KeyExpr: ")
                                 .append(this->as_string_view())
                                 .append(" with ")
                                 .append(other.as_string_view()));
        return k;
    }

    /// @brief Check if 2 key expressions intersect.
    ///
    /// @return true if there is at least one non-empty key that is contained in both key expressions, false otherwise.
    bool intersects(const KeyExpr& other) const { return ::z_keyexpr_intersects(this->loan(), other.loan()); }
#if defined(UNSTABLE)
    ///
    /// Intersection level of 2 key expressions.
    ///
    /// Values:
    /// - **Z_KEYEXPR_INTERSECTION_LEVEL_DISJOINT**:  2 key expression do not intersect.
    /// - **Z_KEYEXPR_INTERSECTION_LEVEL_INTERSECTS**: 2 key expressions intersect, i.e. there exists at least one key
    /// that is included by both.
    /// - **Z_KEYEXPR_INTERSECTION_LEVEL_INCLUDES**: First key expression is the superset of second one.
    /// - **Z_KEYEXPR_INTERSECTION_LEVEL_EQUALS**: 2 key expressions are equal.
    ///
    typedef ::z_keyexpr_intersection_level_t IntersectionLevel;
    /// @brief Returns the relation between `this` and `other` from `this`'s point of view.
    IntersectionLevel relation_to(const KeyExpr& other) { return ::z_keyexpr_relation_to(this->loan(), other.loan()); }
#endif
    /// @brief Check if the string represents a canonical key expression
    static bool is_canon(std::string_view s) { return ::z_keyexpr_is_canon(s.data(), s.size()) == Z_OK; }

    /// @name Operators

    /// @brief Key expression to string equality relation.
    /// @param other a string to compare with.
    /// @return true if the key expression string representation is equal to other, false otherwise.
    bool operator==(std::string_view other) const {
        if (!(this->internal_check())) return false;
        return as_string_view() == other;
    }

    /// @brief Key expression to string inequality relation.
    /// @param other a string to compare with.
    /// @return false if the key expression string representation is equal to other, true otherwise.
    bool operator!=(std::string_view other) const { return !((*this) == other); }

    /// @brief Key expression to string equality relation.
    /// @param other a string to compare with.
    /// @return true if the key expression string representation is equal to other, false otherwise.
    bool operator==(const std::string& other) const {
        if (!(this->internal_check())) return false;
        return as_string_view() == other;
    }

    /// @brief Key expression to string inequality relation.
    /// @param other a string to compare with.
    /// @return false if the key expression string representation is equal to other, true otherwise.
    bool operator!=(const std::string& other) const { return !((*this) == other); }

    /// @brief Key expression to string equality relation.
    /// @param other a null-terminated string to compare with.
    /// @return true if the key expression string representation is equal to other, false otherwise.
    bool operator==(const char* other) const {
        if (!(this->internal_check())) return false;
        return as_string_view() == other;
    }

    /// @brief Key expression to string inequality relation.
    /// @param other a null-terminated string to compare with.
    /// @return false if the key expression string representation is equal to other, true otherwise.
    bool operator!=(const char* other) const { return !((*this) == other); }

    /// @brief Equality relation.
    /// @param other a key expression to compare with.
    /// @return true if both key expressions are equal (i.e. they represent the same set of resources), false otherwise.
    bool operator==(const KeyExpr& other) const { return ::z_keyexpr_equals(this->loan(), other.loan()); }

    /// @brief Inequality relation.
    /// @param other a key expression to compare with.
    /// @return false if both key expressions are equal (i.e. they represent the same set of resources), true otherwise.
    bool operator!=(const KeyExpr& other) const { return !(*this == other); }
};

}  // namespace zenoh