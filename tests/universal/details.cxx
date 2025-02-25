//
// Copyright (c) 2025 ZettaScale Technology
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
//

#include "zenoh.hxx"

using namespace zenoh;

static_assert(detail::is_take_from_loaned_available_v<::z_owned_session_t> == false);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_hello_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_sample_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_reply_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_query_t>);

int main() { return 0; }