//
// Copyright (c) 2025 ZettaScale Technology
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

#include "zenoh.hxx"

#undef NDEBUG
#include <cassert>

using namespace zenoh;

static_assert(detail::is_take_from_loaned_available_v<::z_owned_session_t> == false);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_hello_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_sample_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_reply_t>);
static_assert(detail::is_take_from_loaned_available_v<::z_owned_query_t>);

static void test_string_array_to_vector() {
    ::z_owned_string_array_t array;
    ::z_string_array_new(&array);
    assert(interop::detail::string_array_to_vector(array).empty());
    assert(!::z_internal_string_array_check(&array));

    ::z_string_array_new(&array);
    const std::vector<std::string> expected{"eth0", "", std::string("a\0b", 3), "lo"};
    for (const auto& text : expected) {
        ::z_view_string_t view;
        assert(::z_view_string_from_substr(&view, text.data(), text.size()) == Z_OK);
        ::z_string_array_push_by_copy(::z_loan_mut(array), ::z_loan(view));
    }
    const auto result = interop::detail::string_array_to_vector(array);
    assert(!::z_internal_string_array_check(&array));
    assert(result == expected);
}

int main() {
    test_string_array_to_vector();
    return 0;
}
