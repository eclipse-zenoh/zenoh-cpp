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
#include "sample.hxx"
#include "bytes.hxx"
#include "id.hxx"


namespace zenoh {

/// @brief Reply error data.
class ReplyError : public Owned<::z_owned_reply_err_t> {
public:
    using Owned::Owned;
    /// @name Methods

    /// @brief The payload of this error.
    /// @return Error payload.
    decltype(auto) get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_reply_err_payload(this->loan())); }

    /// @brief The encoding of this error.
    /// @return Error encoding.
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Encoding>(::z_reply_err_encoding(this->loan())); }
};

/// A reply from queryable to ``Session::get`` operation.
class Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Check if the reply is OK (and contains a sample).
    /// @return ``true`` if the reply is OK, ``false`` if contains a error.
    bool is_ok() const { return ::z_reply_is_ok(this->loan()); }

    /// @brief Get the reply value. Will throw a ZException if ``Reply::is_ok`` returns ``false``.
    /// @return Reply sample.
    decltype(auto) get_ok() const {
        if (!::z_reply_is_ok(this->loan())) {
            throw ZException("Reply data sample was requested, but reply contains error", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Sample>(::z_reply_ok(this->loan()));
    }

    /// @brief Get the reply error. Will throw a ZException if ``Reply::is_ok`` returns ``true``.
    /// @return Reply error.
    decltype(auto) get_err() const {
        if (::z_reply_is_ok(this->loan())) {
            throw ZException("Reply error was requested, but reply contains data sample", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<ReplyError>(::z_reply_err(this->loan()));
    }

#if defined(UNSTABLE)
    /// @brief Get the id of the Zenoh instance that issued this reply.
    /// @return Zenoh instance id, or an empty optional if the id was not set.
    std::optional<Id> get_replier_id() const {
        ::z_id_t z_id;
        if (::z_reply_replier_id(this->loan(), &z_id)) {
            return Id(z_id);
        }
        return {};
    }
    #endif
};

}