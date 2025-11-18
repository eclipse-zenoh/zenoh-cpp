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
// This file contains structures and classes API without implementations
//

#pragma once
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1)
#include "base.hxx"
#include "interop.hxx"

namespace zenoh {
/// A Zenoh Cancellation token that can be used.
/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
/// release.
class CancellationToken : public Owned<::z_owned_cancellation_token_t> {
   public:
    /// @name Constructors

    /// @brief Construct a new cancellation token.
    CancellationToken() : Owned(nullptr) { ::z_cancellation_token_new(interop::as_owned_c_ptr(*this)); }

    /// @brief Copy constructor.
    ///
    /// Cancelling a token also cancels all of its copies.
    CancellationToken(CancellationToken& other) : Owned(nullptr) {
        ::z_cancellation_token_clone(interop::as_owned_c_ptr(*this), interop::as_loaned_c_ptr(other));
    }

    /// @name Operators
    /// @brief Assignment operator.
    CancellationToken& operator=(const CancellationToken& other) {
        if (this != &other) {
            ::z_drop(interop::as_moved_c_ptr(*this));
            ::z_cancellation_token_clone(interop::as_owned_c_ptr(*this), interop::as_loaned_c_ptr(other));
        }
        return *this;
    };

    /// @name Methods

    /// @brief Interrupt all currently running get calls, to which clones of the token were passed.
    void cancel(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(::z_cancellation_token_cancel(interop::as_loaned_c_ptr(*this)), err,
                             "Failed to perform Cancellation::cancel");
    }

    /// @brief Verify if token was cancelled.
    ///
    /// @return ``true`` if token was cancelled (i.e if ``cancel`` was called on it or one of its copies), ``false``
    /// otherwise.
    bool is_cancelled() { return ::z_cancellation_token_is_cancelled(interop::as_loaned_c_ptr(*this)); }
};
}  // namespace zenoh
#endif
