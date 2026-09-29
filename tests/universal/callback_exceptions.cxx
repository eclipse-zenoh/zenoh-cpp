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
//

#include <cstdlib>
#include <exception>
#include <memory>
#include <string_view>

#include "zenoh.hxx"
#undef NDEBUG
#include <cassert>

using namespace zenoh;
using namespace zenoh::detail::closures;

// Check the actual adapter signatures, including feature-gated callbacks.
template <class R, class... Args>
constexpr bool is_nothrow_callback(R (*)(Args...) noexcept) {
    return true;
}
template <class R, class... Args>
constexpr bool is_nothrow_callback(R (*)(Args...)) {
    return false;
}

static_assert(is_nothrow_callback(_zenoh_on_drop));
static_assert(is_nothrow_callback(_zenoh_drop_with_context));
static_assert(is_nothrow_callback(_zenoh_on_sample_call));
static_assert(is_nothrow_callback(_zenoh_on_id_call));
static_assert(is_nothrow_callback(_zenoh_on_hello_call));
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
static_assert(is_nothrow_callback(_zenoh_on_reply_call));
#endif
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1
static_assert(is_nothrow_callback(_zenoh_on_query_call));
#endif
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1
static_assert(is_nothrow_callback(_zenoh_on_status_change_call));
#endif
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_CONNECTIVITY == 1)
static_assert(is_nothrow_callback(_zenoh_on_transport_call));
static_assert(is_nothrow_callback(_zenoh_on_link_call));
static_assert(is_nothrow_callback(_zenoh_on_transport_event_call));
static_assert(is_nothrow_callback(_zenoh_on_link_event_call));
#endif
#if defined(Z_FEATURE_UNSTABLE_API) && (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_ADVANCED_SUBSCRIPTION == 1)
static_assert(is_nothrow_callback(_zenoh_on_miss_detected_call));
#endif
#if defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
static_assert(is_nothrow_callback(shm::segment::closures::_z_cpp_shm_segment_map_fn));
static_assert(is_nothrow_callback(shm::segment::closures::_z_cpp_shm_segment_drop_fn));
static_assert(is_nothrow_callback(shm::client::closures::_z_cpp_shm_client_attach_fn));
static_assert(is_nothrow_callback(shm::client::closures::_z_cpp_shm_client_id_fn));
static_assert(is_nothrow_callback(shm::client::closures::_z_cpp_shm_client_drop_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_drop_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_alloc_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_free_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_defragment_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_available_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_layout_for_fn));
static_assert(is_nothrow_callback(shm::provider_backend::closures::_z_cpp_shm_provider_backend_id_fn));
static_assert(is_nothrow_callback(shm::provider::closures::_z_precomputed_layout_async_interface_result_fn));
static_assert(is_nothrow_callback(shm::provider::closures::_z_precomputed_layout_async_interface_drop_fn));
#endif

static void test_success() {
    int calls = 0;
    int drops = 0;
    auto capture = std::make_shared<int>(42);
    std::weak_ptr<int> weak = capture;
    auto call = [&](const Id&) { ++calls; };
    auto drop = [&, capture = std::move(capture)] { ++drops; };
    using Context = Closure<decltype(call), decltype(drop), void, const Id&>;
    auto* context = Context::into_context(std::move(call), std::move(drop));
    z_id_t id{};
    _zenoh_on_id_call(&id, context);
    _zenoh_on_drop(context);
    assert(calls == 1 && drops == 1);
    assert(weak.expired());

    auto bytes_drop = [&] { ++drops; };
    _zenoh_drop_with_context(nullptr, Droppable<decltype(bytes_drop)>::into_context(std::move(bytes_drop)));
    assert(drops == 2);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        test_success();
        return 0;
    }
#ifdef __cpp_exceptions
    // A distinct exit code proves that the adapter called terminate, rather than
    // leaking an exception to its caller or merely crashing.
    std::set_terminate([] { std::_Exit(77); });
    try {
        std::string_view test(argv[1]);
        if (test == "call") {
            auto call = [](const Id&) { throw 1; };
            auto drop = [] {};
            using Context = Closure<decltype(call), decltype(drop), void, const Id&>;
            auto* context = Context::into_context(std::move(call), std::move(drop));
            z_id_t id{};
            _zenoh_on_id_call(&id, context);
            _zenoh_on_drop(context);
        } else if (test == "shm_map") {
#if defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
            struct Segment : CppShmSegment {
                uint8_t* map(z_chunk_id_t) override { throw 1; }
            } segment;
            shm::segment::closures::_z_cpp_shm_segment_map_fn(0, &segment);
#else
            return 125;
#endif
        } else if (test == "drop" || test == "bytes_drop") {
            auto drop = [] { throw 1; };
            auto* context = Droppable<decltype(drop)>::into_context(std::move(drop));
            if (test == "drop")
                _zenoh_on_drop(context);
            else
                _zenoh_drop_with_context(nullptr, context);
        }
    } catch (...) {
        return 78;
    }
    return 79;
#else
    (void)argv;
    return 125;
#endif
}
