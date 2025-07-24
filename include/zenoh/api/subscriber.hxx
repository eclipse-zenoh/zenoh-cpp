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

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_SUBSCRIPTION == 1

#include <utility>

#include "base.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif

namespace zenoh {

class Session;
namespace detail {
class SubscriberBase : public Owned<::z_owned_subscriber_t> {
   protected:
    SubscriberBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    SubscriberBase(::z_owned_subscriber_t* s) : Owned(s){};

   public:
    /// @brief Get the key expression of the subscriber.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_subscriber_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the subscriber.
    /// @return id of this subscriber.
    /// @note Zenoh-c only.
    EntityGlobalId get_id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(::z_subscriber_id(interop::as_loaned_c_ptr(*this)));
    }
#endif
};

}  // namespace detail

template <class Handler>
class Subscriber;
template <>
class Subscriber<void> : public detail::SubscriberBase {
   protected:
    using SubscriberBase::SubscriberBase;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_subscriber(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare subscriber");
    }
};

/// @brief A Zenoh subscriber. Destroying or undeclaring subscriber cancels the subscription.

/// Constructed by ``Session::declare_subscriber`` method.
/// @tparam Handler streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class Subscriber : public detail::SubscriberBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream subscriber from callback subscriber and handler.
    ///
    /// @param s callback subscriber, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `s`. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    Subscriber(Subscriber<void>&& s, Handler handler)
        : SubscriberBase(interop::as_owned_c_ptr(s)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare subscriber, and return its handler, which can still be used to process any messages received
    /// prior to undeclaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_subscriber(::z_move(this->_0)), err, "Failed to undeclare subscriber");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to subscriber data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_owned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const Subscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::SubscriberBase&>(s)),
                          as_owned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_loaned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const Subscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::detail::SubscriberBase&>(s)),
                          as_loaned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(Subscriber<Handler>& s) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::SubscriberBase&>(s)),
                          as_moved_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of subscriber and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<Subscriber<Handler>>& s) -> decltype(as_moved_c_ptr(s.value())) {
    if (!s.has_value()) {
        return as_moved_c_ptr(s.value());
    } else {
        return {};
    }
}

/// @brief Move subscriber and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(Subscriber<Handler>&& s) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::SubscriberBase&>(s))),
                          move_to_c_obj(std::move(const_cast<Handler&>(s))));
}
}  // namespace interop

}  // namespace zenoh
#endif