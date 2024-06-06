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

//
// This file contains structures and classes API without implementations
//

#pragma once

#include "../zenohc.hxx"
#include "../detail/interop.hxx"
#include "closures.hxx"
#include "../api/sample.hxx"
#include "../api/reply.hxx"
#include "../api/query.hxx"
#include "../api/id.hxx"
#include "../api/hello.hxx"

/// Ensure that function pointers are defined with extern C linkage
namespace zenoh::detail::closures {
extern "C" {
    inline void _zenoh_on_drop(void* context) {
        IDroppable::delete_from_context(context);
    }

    inline void _zenoh_on_reply_call(const ::z_loaned_reply_t* reply, void* context) {
        IClosure<void, const Reply&>::call_from_context(context, detail::as_owned_cpp_obj<Reply>(reply));
    }

    inline void _zenoh_on_sample_call(const ::z_loaned_sample_t* sample, void* context) {
        IClosure<void, const Sample&>::call_from_context(context, detail::as_owned_cpp_obj<Sample>(sample));
    }

    inline void _zenoh_on_query_call(const ::z_loaned_query_t* query, void* context) {
        IClosure<void, const Query&>::call_from_context(context, detail::as_owned_cpp_obj<Query>(query));
    }

    inline void _zenoh_on_id_call(const ::z_id_t* z_id, void* context) {
        IClosure<void, const Id&>::call_from_context(context, detail::as_copyable_cpp_obj<Id>(z_id));
    }

    inline void _zenoh_on_hello_call(const ::z_loaned_hello_t* hello, void* context) {
        IClosure<void, const Hello&>::call_from_context(context, detail::as_owned_cpp_obj<Hello>(hello));
    }
}
}
