#pragma once
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

#include "private/api.hxx"
#include "private/zenohc.hxx"

#include <string_view>
#include <string>
#include <utility>
#include <vector>

namespace zenoh::serde {

Bytes serialize(const char* c) {
    Bytes b(nullptr);
    ::z_bytes_encode_from_string(detail::as_owned_c_ptr(b), c);
    return b;
}

Bytes serialize(const std::pair<const uint8_t*, size_t>& s, ZError* e = nullptr) {
    Bytes b(nullptr);
    ::z_bytes_encode_from_slice(detail::as_owned_c_ptr(b), s.first, s.second);
    return b;
}

Bytes serialize(const std::string& s) {
    Bytes b(nullptr);
    serialize(s.data());
    return b;
}

Bytes serialize(const std::vector<uint8_t>& s) {
    Bytes b(nullptr);
    serialize(std::make_pair<const uint8_t*, size_t>(s.data(), s.size()));
    return b;
}

template<>
std::string deserialize<std::string>(const Bytes& b, ZError* err) {
    auto reader = b.reader();
    std::string s(b.size(), '0');
    reader.read(reinterpret_cast<uint8_t*>(s.data()), s.size());
    return s;
}

template<>
std::vector<uint8_t> deserialize<std::vector<uint8_t>>(const Bytes& b, ZError* err) {
    auto reader = b.reader();
    std::vector<uint8_t> v(b.size());
    reader.read(v.data(), b.size());
    return v;
}

}