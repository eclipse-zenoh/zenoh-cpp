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
#include "interop.hxx"

namespace zenoh {

class Session;

/// @brief A liveliness token that can be used to provide the network with information about connectivity to its
/// declarer.
///
/// When constructed, a PUT sample will be received by liveliness subscribers on intersecting key expressions.
///
/// A DELETE on the token's key expression will be received by subscribers if the token is destroyed, or if connectivity
/// between the subscriber and the token's creator is lost.
class LivelinessToken : public Owned<::z_owned_liveliness_token_t> {
    LivelinessToken(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// Undeclares liveliness token, resetting it to gravestone state.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_liveliness_undeclare_token(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare liveliness token");
    }
};

}  // namespace zenoh