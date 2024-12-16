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
#include "session_ext.hxx"

namespace zenoh::ext {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
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

[[nodiscard]] PublicationCache SessionExt::declare_publication_cache(const KeyExpr& key_expr,
                                                                     PublicationCacheOptions&& options,
                                                                     ZResult* err) const {
    ::ze_publication_cache_options_t opts;
    ze_publication_cache_options_default(&opts);
    opts.queryable_prefix = interop::as_loaned_c_ptr(options.queryable_prefix);
#if defined(Z_FEATURE_UNSTABLE_API)
    opts.queryable_origin = options.queryable_origin;
#endif
    opts.queryable_complete = options.queryable_complete;
    opts.history = options.history;
    opts.resources_limit = options.resources_limit;
    ext::PublicationCache p = interop::detail::null<ext::PublicationCache>();
    ZResult res = ::ze_declare_publication_cache(interop::as_loaned_c_ptr(this->_session), interop::as_owned_c_ptr(p),
                                                 interop::as_loaned_c_ptr(key_expr), &opts);
    __ZENOH_RESULT_CHECK(res, err, "Failed to declare Publication Cache");
    return p;
}

void SessionExt::declare_background_publication_cache(const KeyExpr& key_expr, PublicationCacheOptions&& options,
                                                      ZResult* err) const {
    ::ze_publication_cache_options_t opts;
    ze_publication_cache_options_default(&opts);
    opts.queryable_prefix = interop::as_loaned_c_ptr(options.queryable_prefix);
#if defined(Z_FEATURE_UNSTABLE_API)
    opts.queryable_origin = options.queryable_origin;
#endif
    opts.queryable_complete = options.queryable_complete;
    opts.history = options.history;
    opts.resources_limit = options.resources_limit;
    ZResult res = ::ze_declare_background_publication_cache(interop::as_loaned_c_ptr(this->_session),
                                                            interop::as_loaned_c_ptr(key_expr), &opts);
    __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Publication Cache");
}

}  // namespace zenoh::ext
#endif
