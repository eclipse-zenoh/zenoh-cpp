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

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1

#include "base.hxx"
#include "bytes.hxx"
#include "interop.hxx"
#include "sample.hxx"
#if defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif

namespace zenoh {

/// @brief Reply error data.
class ReplyError : public Owned<::z_owned_reply_err_t> {
   public:
    /// @name Methods

    /// @brief The payload of this error.
    /// @return error payload.
    const Bytes& get_payload() const {
        return interop::as_owned_cpp_ref<Bytes>(::z_reply_err_payload(interop::as_loaned_c_ptr(*this)));
    }
#if defined(ZENOHCXX_ZENOHC)
    /// @brief The payload of this error.
    /// @return error payload.
    Bytes& get_payload() {
        return interop::as_owned_cpp_ref<Bytes>(::z_reply_err_payload_mut(interop::as_loaned_c_ptr(*this)));
    }
#endif

    /// @brief The encoding of this error.
    /// @return error encoding.
    const Encoding& get_encoding() const {
        return interop::as_owned_cpp_ref<Encoding>(::z_reply_err_encoding(interop::as_loaned_c_ptr(*this)));
    }
};

/// A reply from queryable to ``Session::get`` operation.
class Reply : public Owned<::z_owned_reply_t> {
    Reply(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Check if the reply is OK (and contains a sample).
    /// @return ``true`` if the reply is OK, ``false`` if contains a error.
    bool is_ok() const { return ::z_reply_is_ok(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the reply sample. Will throw a ZException if ``Reply::is_ok`` returns ``false``.
    /// @return reply sample.
    const Sample& get_ok() const {
        if (!::z_reply_is_ok(interop::as_loaned_c_ptr(*this))) {
            throw ZException("Reply data sample was requested, but reply contains error", Z_EINVAL);
        }
        return interop::as_owned_cpp_ref<Sample>(::z_reply_ok(interop::as_loaned_c_ptr(*this)));
    }

#if defined(ZENOHCXX_ZENOHC)
    // TODO: this is temporary disabled because of this: https://github.com/eclipse-zenoh/zenoh-c/pull/718
    // /// @brief Get the mutable reply sample. Will throw a ZException if ``Reply::is_ok`` returns ``false``.
    // /// @return reply sample.
    // Sample& get_ok() {
    //     if (!::z_reply_is_ok(interop::as_loaned_c_ptr(*this))) {
    //         throw ZException("Reply data sample was requested, but reply contains error", Z_EINVAL);
    //     }
    //     return interop::as_owned_cpp_ref<Sample>(::z_reply_ok_mut(interop::as_loaned_c_ptr(*this)));
    // }
#endif

    /// @brief Get the reply error. Will throw a ZException if ``Reply::is_ok`` returns ``true``.
    /// @return reply error.
    const ReplyError& get_err() const {
        if (::z_reply_is_ok(interop::as_loaned_c_ptr(*this))) {
            throw ZException("Reply error was requested, but reply contains data sample", Z_EINVAL);
        }
        return interop::as_owned_cpp_ref<ReplyError>(::z_reply_err(interop::as_loaned_c_ptr(*this)));
    }

#if defined(ZENOHCXX_ZENOHC)
    // TODO: this is temporary disabled because of this: https://github.com/eclipse-zenoh/zenoh-c/pull/718
    // /// @brief Get the mutable reply error. Will throw a ZException if ``Reply::is_ok`` returns ``true``.
    // /// @return reply error.
    // ReplyError& get_err() {
    //     if (::z_reply_is_ok(interop::as_loaned_c_ptr(*this))) {
    //         throw ZException("Reply error was requested, but reply contains data sample", Z_EINVAL);
    //     }
    //     return interop::as_owned_cpp_ref<ReplyError>(::z_reply_err_mut(interop::as_loaned_c_ptr(*this)));
    // }
#endif

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the Zenoh entity that issued this reply.
    /// @return Zenoh entity global id, or an empty optional if the id was not set.
    std::optional<EntityGlobalId> get_replier_id() const {
        ::z_entity_global_id_t z_entity_global_id;
        if (::z_reply_replier_id(interop::as_loaned_c_ptr(*this), &z_entity_global_id)) {
            return interop::into_copyable_cpp_obj<EntityGlobalId>(z_entity_global_id);
        }
        return {};
    }
#endif

    /// @brief Construct a shallow copy of this Reply.
    ///
    /// The reply will be sent only when the last clone is destroyed.
    Reply clone() const {
        Reply reply(zenoh::detail::null_object);
        ::z_reply_clone(&reply._0, interop::as_loaned_c_ptr(*this));
        return reply;
    };
};

}  // namespace zenoh
#endif
