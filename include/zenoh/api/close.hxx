
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

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)

namespace zenoh {

/// A close operation handle.
class CloseHandle : public Owned<::zc_owned_concurrent_close_handle_t> {
    friend class Session;
    CloseHandle(zenoh::detail::null_object_t) : Owned(nullptr){};

   public:
    /// @brief Blocks until corresponding close operation completes.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void wait(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zc_concurrent_close_handle_wait(interop::as_moved_c_ptr(*this)), err,
                             "Failed to wait for close operation");
    }
};

}  // namespace zenoh

#endif
