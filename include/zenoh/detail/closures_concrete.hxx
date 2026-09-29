//
// Copyright (c) 2023 ZettaScale Technology
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

#include "../api/hello.hxx"
#include "../api/id.hxx"
#include "../api/interop.hxx"
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_CONNECTIVITY == 1)
#include "../api/link.hxx"
#include "../api/link_event.hxx"
#endif
#include "../api/query.hxx"
#include "../api/reply.hxx"
#include "../api/sample.hxx"
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_CONNECTIVITY == 1)
#include "../api/transport.hxx"
#include "../api/transport_event.hxx"
#endif
#include "../zenohc.hxx"
#include "closures.hxx"

// Ensure that function pointers are defined with extern C linkage
namespace zenoh::detail::closures {
extern "C" {
inline void _zenoh_on_drop(void* context) noexcept { IDroppable::delete_from_context(context); }
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
inline void _zenoh_on_reply_call(::z_loaned_reply_t* reply, void* context) noexcept {
    IClosure<void, Reply&>::call_from_context(context, interop::as_owned_cpp_ref<Reply>(reply));
}
#endif
inline void _zenoh_on_sample_call(::z_loaned_sample_t* sample, void* context) noexcept {
    IClosure<void, Sample&>::call_from_context(context, interop::as_owned_cpp_ref<Sample>(sample));
}
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1
inline void _zenoh_on_query_call(::z_loaned_query_t* query, void* context) noexcept {
    IClosure<void, Query&>::call_from_context(context, interop::as_owned_cpp_ref<Query>(query));
}
#endif
inline void _zenoh_on_id_call(const ::z_id_t* z_id, void* context) noexcept {
    IClosure<void, const Id&>::call_from_context(context, interop::as_copyable_cpp_ref<Id>(z_id));
}

inline void _zenoh_on_hello_call(::z_loaned_hello_t* hello, void* context) noexcept {
    IClosure<void, Hello&>::call_from_context(context, interop::as_owned_cpp_ref<Hello>(hello));
}
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_CONNECTIVITY == 1)
inline void _zenoh_on_transport_call(::z_loaned_transport_t* transport, void* context) noexcept {
    IClosure<void, Transport&>::call_from_context(context, interop::as_owned_cpp_ref<Transport>(transport));
}
inline void _zenoh_on_link_call(::z_loaned_link_t* link, void* context) noexcept {
    IClosure<void, Link&>::call_from_context(context, interop::as_owned_cpp_ref<Link>(link));
}
inline void _zenoh_on_transport_event_call(::z_loaned_transport_event_t* event, void* context) noexcept {
    IClosure<void, TransportEvent&>::call_from_context(context, interop::as_owned_cpp_ref<TransportEvent>(event));
}
inline void _zenoh_on_link_event_call(::z_loaned_link_event_t* event, void* context) noexcept {
    IClosure<void, LinkEvent&>::call_from_context(context, interop::as_owned_cpp_ref<LinkEvent>(event));
}
#endif
}
}  // namespace zenoh::detail::closures
