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
#include "../../detail/closures_concrete.hxx"
#include "../base.hxx"
#include "../interop.hxx"
#include "../keyexpr.hxx"
#include "../sample.hxx"

namespace zenoh::ext {

/// @warning This API is deprecated. Please use zenoh::ext::AdvancedPublisher.
/// @brief A Zenoh publication cache.
///
/// Used to store publications on intersecting key expressions. Can be queried later via `zenoh::Session::get` to
/// retrieve this data.
/// @note Zenoh-c only
class PublicationCache : public Owned<::ze_owned_publication_cache_t> {
    PublicationCache(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Get the key expression of the publication cache.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::ze_publication_cache_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Undeclare publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_publication_cache(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Publication Cache");
    }
};

}  // namespace zenoh::ext
#endif
