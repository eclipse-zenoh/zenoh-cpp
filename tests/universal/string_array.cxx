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

#include "zenoh/detail/string_array.hxx"

#include <cstdlib>
#include <new>
#include <string>
#include <type_traits>
#include <vector>
#undef NDEBUG
#include <cassert>

using zenoh::detail::StringArray;

static_assert(!std::is_copy_constructible_v<StringArray>);
static_assert(!std::is_copy_assignable_v<StringArray>);
static_assert(!std::is_move_constructible_v<StringArray>);

#ifdef __cpp_exceptions
// Only C++ allocations are fault-injected. C array construction uses the backend allocator.
static thread_local int allocations_before_failure = -1;

void* operator new(std::size_t size) {
    if (allocations_before_failure == 0) throw std::bad_alloc();
    if (allocations_before_failure > 0) --allocations_before_failure;
    if (void* ptr = std::malloc(size == 0 ? 1 : size)) return ptr;
    throw std::bad_alloc();
}
void operator delete(void* ptr) noexcept { std::free(ptr); }
#ifdef __cpp_sized_deallocation
void operator delete(void* ptr, std::size_t) noexcept { std::free(ptr); }
#endif
#endif

static void append(StringArray& array, const char* data, size_t len) {
    z_view_string_t view;
    assert(z_view_string_from_substr(&view, data, len) == Z_OK);
    z_string_array_push_by_copy(z_loan_mut(array.value), z_loan(view));
}

static constexpr char first[] = "interface-name-long-enough-to-require-a-string-allocation";
static constexpr char second[] = "another-long-interface-name-with-an-embedded\0null";

static void populate(StringArray& array) {
    append(array, first, sizeof(first) - 1);
    append(array, second, sizeof(second) - 1);
}

int main() {
    {
        StringArray empty;
        assert(empty.copy().empty());
    }
    std::vector<std::string> result;
    {
        StringArray array;
        populate(array);
        result = array.copy();
    }
    // Copies remain valid after the C array is dropped and preserve embedded nulls.
    assert(result.size() == 2);
    assert(result[0] == std::string(first, sizeof(first) - 1));
    assert(result[1] == std::string(second, sizeof(second) - 1));

#ifdef __cpp_exceptions
    // Fail vector reservation, then each string allocation. The guard must unwind
    // in every case; running this test under a leak checker also checks C ownership.
    for (int failure = 0; failure < 3; ++failure) {
        bool caught = false;
        try {
            StringArray array;
            populate(array);
            allocations_before_failure = failure;
            auto copy = array.copy();
            allocations_before_failure = -1;
        } catch (const std::bad_alloc&) {
            allocations_before_failure = -1;
            caught = true;
        }
        assert(caught);
    }
#endif
}
