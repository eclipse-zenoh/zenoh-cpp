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

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1

#include "base.hxx"
#include "enums.hxx"
#include "interop.hxx"

namespace zenoh {

/// Replies consolidation mode to apply on replies of get operation.
class QueryConsolidation : public Copyable<::z_query_consolidation_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Create a new default ``QueryConsolidation`` value.
    QueryConsolidation() : Copyable(::z_query_consolidation_default()) {}

    /// @brief Create a new ``QueryConsolidation`` value with the given consolidation mode.
    /// @param v ``zenoh::ConsolidationMode`` value.
    QueryConsolidation(ConsolidationMode v) : Copyable({v}) {}

    /// @name Methods

    /// @name Operators

    /// @brief Equality relation.
    /// @param other a value to compare with.
    /// @return ``true`` if the two values are equal (have the same consolidation mode).
    bool operator==(const QueryConsolidation& other) const { return this->_0.mode == other._0.mode; }

    /// @brief Inequality relation.
    /// @param other a value to compare with.
    /// @return ``true`` if the two values are not equal (have different consolidation mode)
    bool operator!=(const QueryConsolidation& other) const { return !operator==(other); }
};
}  // namespace zenoh
#endif