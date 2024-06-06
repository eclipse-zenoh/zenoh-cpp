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

//
// This file contains structures and classes API without implementations
//

#pragma once

#include "base.hxx"
#include "../detail/interop.hxx"

#include "bytes.hxx"
#include "encoding.hxx"

namespace zenoh {

/// A zenoh value. Contans refrence to data and it's encoding
class Value : public Owned<::z_owned_value_t> {
public:
    using Owned::Owned;
    /// @name Methods

    /// @brief The payload of this value
    /// @return ``Bytes`` object
    decltype(auto) get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_value_payload(this->loan())); }

    /// @brief The encoding of this value
    /// @return ``Encoding`` object
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Encoding>(::z_value_encoding(this->loan())); }
};
}