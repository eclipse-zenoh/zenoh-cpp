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
#include "../zenohc.hxx"
#include <string_view>

namespace zenoh {


/// ``zenoh::Sample`` kind values.
///
/// Values:
///
///  - **Z_SAMPLE_KIND_PUT**: The Sample was issued by a "put" operation.
///  - **Z_SAMPLE_KIND_DELETE**: The Sample was issued by a "delete" operation.
typedef ::z_sample_kind_t SampleKind;

///   Consolidation mode values.
///
///   Values:
///       - **Z_CONSOLIDATION_MODE_AUTO**: Let Zenoh decide the best consolidation mode depending on the query
///       selector.
///       - **Z_CONSOLIDATION_MODE_NONE**: No consolidation is applied. Replies may come in any order and any
///       number.
///       - **Z_CONSOLIDATION_MODE_MONOTONIC**: It guarantees that any reply for a given key expression will be
///       monotonic in time
///           w.r.t. the previous received replies for the same key expression. I.e., for the same key expression
///           multiple replies may be received. It is guaranteed that two replies received at t1 and t2 will have
///           timestamp ts2 > ts1. It optimizes latency.
///       - **Z_CONSOLIDATION_MODE_LATEST**: It guarantees unicity of replies for the same key expression.
///           It optimizes bandwidth.
typedef ::z_consolidation_mode_t ConsolidationMode;

/// Reliability values.
///
/// Values:
///  - **Z_RELIABILITY_BEST_EFFORT**: Defines reliability as "best effort"
///  - **Z_RELIABILITY_RELIABLE**: Defines reliability as "reliable"
typedef ::z_reliability_t Reliability;

///  Congestion control values.
///
///   Values:
///    - **Z_CONGESTION_CONTROL_BLOCK**: Defines congestion control as "block". Messages are not dropped in case of
///       congestion control
///    - **Z_CONGESTION_CONTROL_DROP**: Defines congestion control as "drop". Messages are dropped in case of
///    congestion control
///
typedef ::z_congestion_control_t CongestionControl;

/// Priority of Zenoh messages values.
///
/// Values:
/// - **Z_PRIORITY_REAL_TIME**: Priority for "realtime" messages.
/// - **Z_PRIORITY_INTERACTIVE_HIGH**: Highest priority for "interactive" messages.
/// - **Z_PRIORITY_INTERACTIVE_LOW**: Lowest priority for "interactive" messages.
/// - **Z_PRIORITY_DATA_HIGH**: Highest priority for "data" messages.
/// - **Z_PRIORITY_DATA**: Default priority for "data" messages.
/// - **Z_PRIORITY_DATA_LOW**: Lowest priority for "data" messages.
/// - **Z_PRIORITY_BACKGROUND**: Priority for "background traffic" messages.
typedef ::z_priority_t Priority;

/// Query target values.
///
/// Values:
/// - **Z_QUERY_TARGET_BEST_MATCHING**: The nearest complete queryable if any else all matching queryables.
/// - **Z_QUERY_TARGET_ALL**: All matching queryables.
/// - **Z_QUERY_TARGET_ALL_COMPLETE**: A set of complete queryables.
typedef ::z_query_target_t QueryTarget;

/// @brief Enum indicating type of Zenoh entity.
typedef ::z_whatami_t WhatAmI;

/// @brief Flag indicating type of Zenoh entities to scout for.
typedef ::z_what_t What;

/// @brief Get a human-readable representation of the given ``zenoh::WhatAmI``
/// flag.
/// @param whatami the ``zenoh::WhatAmI`` flag.
/// @return a string representation of the given flag.
inline std::string_view whatami_as_str(WhatAmI whatami) {
    ::z_view_string_t str_out;
    ::z_whatami_to_view_string(whatami, &str_out);
    return std::string_view(::z_string_data(::z_loan(str_out)), ::z_string_len(::z_loan(str_out)));
}

#ifdef ZENOHCXX_ZENOHC
/// The locality of samples to be received by subscribers or targeted by publishers.
///
/// Values:
/// - **ZCU_LOCALITY_ANY**:  Any.
/// - **ZCU_LOCALITY_SESSION_LOCAL**: Only from local sessions.
/// - **ZCU_LOCALITY_SESSION_REMOTE**: Only from remote sessions.
typedef ::zcu_locality_t Locality;
#endif

}