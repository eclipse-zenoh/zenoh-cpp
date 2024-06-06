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
#include "value.hxx"


namespace zenoh {
/// A reply from queryable to ``Session::get`` operation
class Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Check if the reply is OK
    /// @return true if the reply is OK, false if contains a error
    bool is_ok() const { return ::z_reply_is_ok(this->loan()); }

    /// @brief Get the reply value. Will throw an exception if ``Reply::is_ok`` returns false.
    /// @return Reply sample.
    decltype(auto) get_ok() const {
        if (!::z_reply_is_ok(this->loan())) {
            throw ZException("Reply data sample was requested, but reply contains error", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Sample>(::z_reply_ok(this->loan()));
    }

    /// @brief Get the reply error. Will throw an exception if ``Reply::is_ok`` returns false.
    /// @return Reply error.
    decltype(auto) get_err() const {
        if (::z_reply_is_ok(this->loan())) {
            throw ZException("Reply error was requested, but reply contains data sample", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Value>(::z_reply_err(this->loan()));
    }
};

}