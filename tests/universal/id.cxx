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

#include "zenoh.hxx"
#undef NDEBUG
#include <cassert>
#include <type_traits>

using namespace zenoh;

static void test_from_bytes() {
    static_assert(sizeof(Id) == sizeof(::z_id_t));
    static_assert(alignof(Id) == alignof(::z_id_t));
    static_assert(!std::is_convertible_v<std::array<uint8_t, 16>, Id>);
    std::array<uint8_t, 16> bytes{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ZResult err = Z_EINVAL;
    const Id id(bytes, &err);
    assert(err == Z_OK);
    assert(id.bytes() == bytes);
    assert(id.to_string() == "100f0e0d0c0b0a090807060504030201");
    assert(id == Id(id.bytes()));
    const auto copy = id;
    assert(copy == id);
    bytes.fill(0);
    assert(id.bytes()[0] == 1 && id.bytes()[15] == 16);

    // Single nonzero bytes at either end exercise the full little-endian ID range.
    bytes[0] = 1;
    assert(Id(bytes).bytes() == bytes);
    bytes[0] = 0;
    bytes[15] = 0x80;
    assert(Id(bytes).to_string() == "80000000000000000000000000000000");
}

static void test_zero_id() {
    const std::array<uint8_t, 16> zero{};
    ZResult err = Z_OK;
    const Id invalid(zero, &err);
    assert(err == Z_EINVAL);
    assert(invalid.bytes() == zero);
#ifdef __cpp_exceptions
    bool threw = false;
    try {
        const Id invalid_without_error(zero);
        (void)invalid_without_error;
    } catch (const ZException& e) {
        threw = e.e == Z_EINVAL;
    }
    assert(threw);
#endif
}

int main() {
    test_from_bytes();
    test_zero_id();
}
