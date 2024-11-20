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

#include "bytes.hxx"
#include "enums.hxx"
#include "query_consolidation.hxx"
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif

namespace zenoh {
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
/// @brief Options passed to the ``Session::get`` operation.
struct GetOptions {
    /// @name Fields

    /// @brief The Queryables that should be target of the query.
    QueryTarget target = QueryTarget::Z_QUERY_TARGET_BEST_MATCHING;
    /// @brief The replies consolidation strategy to apply on replies to the query.
    QueryConsolidation consolidation = QueryConsolidation();
    /// @brief The priority of the get message.
    Priority priority = Z_PRIORITY_DEFAULT;
    /// @brief The congestion control to apply when routing get message.
    CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
    /// @brief Whether Zenoh will NOT wait to batch get message with others to reduce the bandwith.
    bool is_express = false;
    /// @brief An optional payload of the query.
    std::optional<Bytes> payload = {};
    /// @brief  An optional encoding of the query payload and/or attachment.
    std::optional<Encoding> encoding = {};
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief The source info for the query.
    std::optional<SourceInfo> source_info = {};

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    ///
    /// @brief The accepted replies for the query.
    /// @note Zenoh-c only.
    ReplyKeyExpr accept_replies = ::zc_reply_keyexpr_default();

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Allowed destination.
    /// @note Zenoh-c only.
    Locality allowed_destination = ::zc_locality_default();
#endif

    /// @brief An optional attachment to the query.
    std::optional<Bytes> attachment = {};
    /// @brief The timeout for the query in milliseconds. 0 means default query timeout from zenoh configuration.
    uint64_t timeout_ms = 0;

    /// @name Methods
    /// @brief Create default option settings.
    static GetOptions create_default() { return {}; }
};
#endif
}  // namespace zenoh