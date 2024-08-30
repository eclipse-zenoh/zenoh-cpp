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

#include <utility>

#include "base.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"

namespace zenoh {

class Session;
namespace detail {
class SubscriberBase : public Owned<::z_owned_subscriber_t> {
   protected:
    SubscriberBase() : Owned(nullptr){};
    SubscriberBase(::z_owned_subscriber_t* s) : Owned(s){};

   public:
#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the subscriber
    /// @note zenoh-c only.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_subscriber_keyexpr(interop::as_loaned_c_ptr(*this)));
    }
#endif
    friend class zenoh::Session;
};

}  // namespace detail

template <class Handler>
class Subscriber;
template <>
class Subscriber<void> : public detail::SubscriberBase {
   protected:
    using SubscriberBase::SubscriberBase;
    friend class Session;
};

/// A Zenoh subscriber. Destroying subscriber cancels the subscription.
/// Constructed by ``Session::declare_subscriber`` method.
/// @tparam Handler Streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class Subscriber : public detail::SubscriberBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream subscriber from callback subscriber and handler.
    ///
    /// @param s Callback subscriber, that should expose data to the handler in its callback.
    /// @param handler Handler to access data exposed by s. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    Subscriber(Subscriber<void>&& s, Handler handler)
        : SubscriberBase(interop::as_owned_c_ptr(s)), _handler(std::move(handler)) {}

    /// @name Methods

#ifdef ZENOHCXX_ZENOHC
    using SubscriberBase::get_keyexpr;
#endif
    /// @brief Return the handler to subscriber data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned C representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_owned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to owned C representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const Subscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::SubscriberBase&>(s)),
                          as_owned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to loaned C representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_loaned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to loaned C representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const Subscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::detail::SubscriberBase&>(s)),
                          as_loaned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to moved C representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_moved_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to moved C representations of subscriber and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<Subscriber<Handler>>& s) -> decltype(as_moved_c_ptr(s.value())) {
    if (!s.has_value()) {
        return as_moved_c_ptr(s.value());
    } else {
        return {};
    }
}

/// @brief Move subscriber and its handler to a pair containing corresponding Zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(Subscriber<Handler>&& s) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::SubscriberBase&>(s))),
                          move_to_c_obj(std::move(const_cast<Handler&>(s))));
}
}  // namespace interop

}  // namespace zenoh