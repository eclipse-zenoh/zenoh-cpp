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
#include <vector>

#include "../zenohc.hxx"

namespace zenoh::detail {

// Own temporary C string-array outputs, including during exception unwinding.
struct StringArray {
    ::z_owned_string_array_t value;

    StringArray() { ::z_string_array_new(&value); }
    ~StringArray() { ::z_drop(::z_move(value)); }
    StringArray(const StringArray&) = delete;
    StringArray& operator=(const StringArray&) = delete;

    std::vector<std::string> copy() const {
        const auto* loan = ::z_loan(value);
        std::vector<std::string> result;
        result.reserve(::z_string_array_len(loan));
        for (size_t i = 0; i < ::z_string_array_len(loan); ++i) {
            const auto* s = ::z_string_array_get(loan, i);
            result.emplace_back(::z_string_data(s), ::z_string_len(s));
        }
        return result;
    }
};

}  // namespace zenoh::detail
