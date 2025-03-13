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
#include "interop.hxx"

namespace zenoh {

/// @brief A Zenoh <a href="https://zenoh.io/docs/manual/abstractions/#key-expression"> key expression </a>.
///
/// Key expression can be registered in the ``zenoh::Session`` object with ``zenoh::Session::declare_keyexpr`` method.
/// The unique id is internally assinged to the key expression string in this case. This allows to reduce bandwith usage
/// when transporting key expressions.

class KeyExpr : public Owned<::z_owned_keyexpr_t> {
    KeyExpr(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Create a new instance from a string.
    ///
    /// @param key_expr string representing key expression.
    /// @param autocanonize if ``true`` the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr(std::string_view key_expr, bool autocanonize = true, ZResult* err = nullptr) : Owned(nullptr) {
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
    /// @param key_expr string representing key expression.
    /// @param autocanonize if ``true`` the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr(const std::string& key_expr, bool autocanonize = true, ZResult* err = nullptr)
        : KeyExpr(static_cast<std::string_view>(key_expr), autocanonize, err){};

    /// @brief Create a new instance from a null-terminated string.
    ///
    /// @param key_expr null-terminated string representing key expression.
    /// @param autocanonize if ``true`` the key_expr will be autocanonized, prior to constructing key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr(const char* key_expr, bool autocanonize = true, ZResult* err = nullptr)
        : KeyExpr(std::string_view(key_expr), autocanonize, err){};

    /// @brief Copy constructor.
    KeyExpr(const KeyExpr& other) : KeyExpr(zenoh::detail::null_object) {
        ::z_keyexpr_clone(&this->_0, interop::as_loaned_c_ptr(other));
    };

    KeyExpr(KeyExpr&& other) = default;

    /// @name Methods
    /// @brief Get underlying key expression string.
    std::string_view as_string_view() const {
        ::z_view_string_t s;
        ::z_keyexpr_as_view_string(interop::as_loaned_c_ptr(*this), &s);
        return std::string_view(reinterpret_cast<const char*>(::z_string_data(::z_loan(s))),
                                ::z_string_len(::z_loan(s)));
    }

    /// @brief Check if a given ``KeyExpr`` includes the other.
    /// @param other the ``KeyExpr`` to compare with
    /// @return ``true`` if current key expression includes ``other``, i.e. contains every key belonging to the
    /// ``other``.
    bool includes(const KeyExpr& other) {
        return ::z_keyexpr_includes(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    }

    /// @brief Construct new key expression by concatenating the current one with a string.
    /// @param s a string to concatenate with the key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a new key expression.
    KeyExpr concat(std::string_view s, ZResult* err = nullptr) const {
        KeyExpr k(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(
            ::z_keyexpr_concat(&k._0, interop::as_loaned_c_ptr(*this), s.data(), s.size()), err,
            std::string("Failed to concatenate KeyExpr: ").append(this->as_string_view()).append(" with ").append(s));
        return k;
    }

    /// @brief Construct new key expression by joining it with another one.
    /// @param other the ``KeyExpr`` to append.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    KeyExpr join(const KeyExpr& other, ZResult* err = nullptr) const {
        KeyExpr k(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(::z_keyexpr_join(&k._0, interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other)),
                             err,
                             std::string("Failed to join KeyExpr: ")
                                 .append(this->as_string_view())
                                 .append(" with ")
                                 .append(other.as_string_view()));
        return k;
    }

    /// @brief Check if 2 key expressions intersect.
    ///
    /// @return ``true`` if there is at least one non-empty key that is contained in both key expressions, ``false``
    /// otherwise.
    bool intersects(const KeyExpr& other) const {
        return ::z_keyexpr_intersects(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    }
#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Intersection level of 2 key expressions.
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
    IntersectionLevel relation_to(const KeyExpr& other) {
        return ::z_keyexpr_relation_to(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    }
#endif
    /// @brief Check if the string represents a canonical key expression
    static bool is_canon(std::string_view s) { return ::z_keyexpr_is_canon(s.data(), s.size()) == Z_OK; }

    /// @name Operators

    /// @brief Key expression to string equality relation.
    /// @param other a string to compare with.
    /// @return ``true`` if the key expression string representation is equal to other, ``false`` otherwise.
    bool operator==(std::string_view other) const { return as_string_view() == other; }

    /// @brief Key expression to string inequality relation.
    /// @param other a string to compare with.
    /// @return ``false`` if the key expression string representation is equal to other, ``true`` otherwise.
    bool operator!=(std::string_view other) const { return !((*this) == other); }

    /// @brief Key expression to string equality relation.
    /// @param other a string to compare with.
    /// @return ``true`` if the key expression string representation is equal to other, ``false`` otherwise.
    bool operator==(const std::string& other) const { return as_string_view() == other; }

    /// @brief Key expression to string inequality relation.
    /// @param other a string to compare with.
    /// @return ``false`` if the key expression string representation is equal to other, ``true`` otherwise.
    bool operator!=(const std::string& other) const { return !((*this) == other); }

    /// @brief Key expression to string equality relation.
    /// @param other a null-terminated string to compare with.
    /// @return ``true`` if the key expression string representation is equal to other, ``false`` otherwise.
    bool operator==(const char* other) const { return as_string_view() == other; }

    /// @brief Key expression to string inequality relation.
    /// @param other a null-terminated string to compare with.
    /// @return ``false`` if the key expression string representation is equal to other, ``true`` otherwise.
    bool operator!=(const char* other) const { return !((*this) == other); }

    /// @brief Equality relation.
    /// @param other a key expression to compare with.
    /// @return ``true`` if both key expressions are equal (i.e. they represent the same set of resources), ``false``
    /// otherwise.
    bool operator==(const KeyExpr& other) const {
        return ::z_keyexpr_equals(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    }

    /// @brief Inequality relation.
    /// @param other a key expression to compare with.
    /// @return ``false`` if both key expressions are equal (i.e. they represent the same set of resources), ``true``
    /// otherwise.
    bool operator!=(const KeyExpr& other) const { return !(*this == other); }

    /// @brief Assignment operator.
    KeyExpr& operator=(const KeyExpr& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_keyexpr_clone(&this->_0, interop::as_loaned_c_ptr(other));
        }
        return *this;
    };

    KeyExpr& operator=(KeyExpr&& other) = default;
};

}  // namespace zenoh