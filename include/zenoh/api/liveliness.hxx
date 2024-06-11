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
#include "../detail/interop.hxx"

#ifdef ZENOHCXX_ZENOHC
namespace zenoh {
class LivelinessToken: public Owned<::zc_owned_liveliness_token_t> {
public:
    using Owned::Owned;

    /// Undeclares liveliness token, resetting it to gravestone state.
    void undeclare(ZError* err = nullptr) && {
        __ZENOH_ERROR_CHECK(
            ::zc_liveliness_undeclare_token(detail::as_owned_c_ptr(*this)),
            err,
            "Failed to undeclare liveliness token"
        );
    }
};
#endif


}