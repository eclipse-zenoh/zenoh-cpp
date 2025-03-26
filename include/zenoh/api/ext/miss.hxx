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

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)

#include "../../detail/closures.hxx"
#include "../base.hxx"
#include "../interop.hxx"
#include "../source_info.hxx"

namespace zenoh::ext {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
/// release.
/// @brief A struct that represents missed samples.
/// @note Zenoh-c only.
struct Miss {
    /// @name Fields

    /// The source of missed samples.
    zenoh::EntityGlobalId source;
    /// The number of missed samples.
    uint32_t nb;
};
}  // namespace zenoh::ext

namespace zenoh::detail::closures {
extern "C" {
inline void _zenoh_on_miss_detected_call(const ::ze_miss_t* miss, void* context) {
    IClosure<void, const zenoh::ext::Miss&>::call_from_context(
        context,
        zenoh::ext::Miss{zenoh::interop::into_copyable_cpp_obj<zenoh::EntityGlobalId>(miss->source), miss->nb});
}
}
}  // namespace zenoh::detail::closures

namespace zenoh::ext {
namespace detail {
class SampleMissListenerBase : public Owned<::ze_owned_sample_miss_listener_t> {
   protected:
    SampleMissListenerBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    SampleMissListenerBase(::ze_owned_sample_miss_listener_t* m) : Owned(m){};
    friend struct zenoh::interop::detail::Converter;
};
}  // namespace detail

template <class Handler>
class SampleMissListener;

template <>
class SampleMissListener<void> : public detail::SampleMissListenerBase {
   protected:
    using SampleMissListenerBase::SampleMissListenerBase;
    friend struct zenoh::interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare sample miss listener.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(zenoh::ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_sample_miss_listener(zenoh::interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Sample Miss Listener");
    }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
/// release.
/// @brief A Zenoh sample miss listener listener. Missed samples can only be detected from advanced publishers, enabling
/// sample miss detection.
///
/// A listener that sends notification when the advanced subscriber misses a sample.
/// Dropping the corresponding subscriber, also drops the listener.
/// @note Zenoh-c only.
template <class Handler>
class SampleMissListener : public detail::SampleMissListenerBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream sample miss listener from callback sample miss listener and handler.
    ///
    /// @param m callback sample miss listener, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `m`. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    SampleMissListener(SampleMissListener<void>&& m, Handler handler)
        : SampleMissListenerBase(zenoh::interop::as_owned_c_ptr(m)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare sample miss listener, and return its handler, which can still be used to process any messages
    /// received prior to undeclaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_sample_miss_listener(::z_move(this->_0)), err,
                             "Failed to undeclare Sample Miss Listener");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to sample miss listener data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};
}  // namespace zenoh::ext

namespace zenoh::interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of sample miss listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(zenoh::ext::SampleMissListener<Handler>& m) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::ext::detail::SampleMissListenerBase&>(m)),
                          as_owned_c_ptr(const_cast<Handler&>(m.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of sample miss listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const zenoh::ext::SampleMissListener<Handler>& m) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::ext::detail::SampleMissListenerBase&>(m)),
                          as_owned_c_ptr(m.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of sample miss listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(zenoh::ext::SampleMissListener<Handler>& m) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::ext::detail::SampleMissListenerBase&>(m)),
                          as_moved_c_ptr(const_cast<Handler&>(m.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of sample miss listener and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<zenoh::ext::SampleMissListener<Handler>>& m) -> decltype(as_moved_c_ptr(m.value())) {
    if (!m.has_value()) {
        return as_moved_c_ptr(m.value());
    } else {
        return {};
    }
}

/// @brief Move sample miss listener and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(zenoh::ext::SampleMissListener<Handler>&& m) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::ext::detail::SampleMissListenerBase&>(m))),
                          move_to_c_obj(std::move(const_cast<Handler&>(m))));
}
}  // namespace zenoh::interop
#endif