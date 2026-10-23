//
// Copyright (c) 2026 ZettaScale Technology
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

#include <string>
#include <string_view>

#include "../api/base.hxx"

namespace zenoh::detail {

// Own temporary C string outputs, including during conversion or stream exceptions.
class String : public Owned<::z_owned_string_t> {
   public:
    String() : Owned(nullptr) {}
    String(const String&) = delete;
    String& operator=(const String&) = delete;

    std::string_view as_string_view() const {
        const auto* loan = ::z_loan(this->_0);
        const auto len = ::z_string_len(loan);
        if (len == 0) return {};
        return std::string_view(::z_string_data(loan), len);
    }

    std::string as_string() const { return std::string(as_string_view()); }
};

}  // namespace zenoh::detail
