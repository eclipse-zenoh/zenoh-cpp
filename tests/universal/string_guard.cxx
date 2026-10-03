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

#include <cstdlib>
#include <cstring>
#include <new>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "zenoh.hxx"
#include "zenoh/detail/string.hxx"
#undef NDEBUG
#include <cassert>

using namespace zenoh;
static_assert(!std::is_copy_constructible_v<detail::String>);
static_assert(!std::is_copy_assignable_v<detail::String>);

#ifdef __cpp_exceptions
// Only C++ allocations are fault-injected. C string construction uses the backend allocator.
static thread_local int allocations_before_failure = -1;

void* operator new(std::size_t size) {
    // MSVC debug containers allocate a two-pointer iterator proxy, even in
    // noexcept constructors. Only fault-inject larger result-storage allocations.
    if (size > 2 * sizeof(void*)) {
        if (allocations_before_failure == 0) {
            allocations_before_failure = -1;
            throw std::bad_alloc();
        }
        if (allocations_before_failure > 0) --allocations_before_failure;
    }
    if (void* ptr = std::malloc(size == 0 ? 1 : size)) return ptr;
    throw std::bad_alloc();
}
void operator delete(void* ptr) noexcept { std::free(ptr); }
#ifdef __cpp_sized_deallocation
void operator delete(void* ptr, std::size_t) noexcept { std::free(ptr); }
#endif
#endif

static constexpr char text[] = "a-string-long-enough-to-require-a-C++-allocation";

static void populate(detail::String& value, int& drops) {
    auto* data = static_cast<char*>(std::malloc(sizeof(text)));
    assert(data != nullptr);
    std::memcpy(data, text, sizeof(text));
    assert(z_string_from_str(
               interop::as_owned_c_ptr(value), data,
               [](void* ptr, void* context) {
                   ++*static_cast<int*>(context);
                   std::free(ptr);
               },
               &drops) == Z_OK);
}

#ifdef __cpp_exceptions
template <class F>
static void allocation_failure(F&& operation) {
    bool caught = false;
    try {
        allocations_before_failure = 0;
        operation();
        allocations_before_failure = -1;
    } catch (const std::bad_alloc&) {
        allocations_before_failure = -1;
        caught = true;
    }
    assert(caught);
}

struct FailingBuffer : std::streambuf {
    std::streamsize xsputn(const char*, std::streamsize) override { throw 42; }
};
#endif

int main() {
    detail::String empty;
    assert(empty.as_string().empty());
    assert(empty.as_string_view().empty());
    int drops = 0;
    std::string copy;
    {
        detail::String value;
        populate(value, drops);
        assert(value.as_string_view() == text);
        copy = value.as_string();
        assert(drops == 0);
    }
    assert(drops == 1 && copy == text);
    {
        detail::String value;
        const char embedded[] = "a\0b";
        assert(z_string_copy_from_substr(interop::as_owned_c_ptr(value), embedded, 3) == Z_OK);
        assert(value.as_string() == std::string(embedded, 3));
    }

    z_id_t raw_id{};
    for (auto& byte : raw_id.id) byte = 0xab;
    const auto id = interop::into_copyable_cpp_obj<Id>(raw_id);
    std::ostringstream output;
    output << id;
    assert(output.str() == id.to_string());
    const std::string encoding_text = "application/python-serialized-object";
    const Encoding encoding(encoding_text);
    assert(encoding.as_string() == encoding_text);
    const auto serialized = ext::serialize(std::string(text));
    assert(ext::deserialize<std::string>(serialized) == text);

#if defined(ZENOHCXX_ZENOHC)
    const auto config = Config::create_default();
    assert(!config.to_string().empty());
    assert(!config.get("transport").empty());
    ZResult err = Z_OK;
    assert(config.get("invalid/key/for/test", &err).empty());
    assert(err != Z_OK);
#endif

#ifdef __cpp_exceptions
    // Directly verify the C deleter runs during stack unwinding.
    allocation_failure([&] {
        detail::String value;
        populate(value, drops);
        auto result = value.as_string();
    });
    assert(drops == 2);
    allocation_failure([&] { id.to_string(); });
    allocation_failure([&] { encoding.as_string(); });
    allocation_failure([&] { ext::deserialize<std::string>(serialized); });
#if defined(ZENOHCXX_ZENOHC)
    allocation_failure([&] { config.to_string(); });
    allocation_failure([&] { config.get("transport"); });
#endif

    FailingBuffer buffer;
    std::ostream stream(&buffer);
    stream.exceptions(std::ios::badbit);
    bool caught = false;
    try {
        detail::String value;
        populate(value, drops);
        stream << value.as_string_view();
    } catch (int error) {
        assert(error == 42);
        caught = true;
    }
    assert(caught && drops == 3);
    stream.clear();
    caught = false;
    try {
        stream << id;
    } catch (int error) {
        assert(error == 42);
        caught = true;
    }
    assert(caught);
#endif
}
