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
#include "../../detail/closures_concrete.hxx"
#include "../base.hxx"
#include "../interop.hxx"
#include "../keyexpr.hxx"
#include "../sample.hxx"

namespace zenoh::ext {

namespace detail {
class QueryingSubscriberBase : public Owned<::ze_owned_querying_subscriber_t> {
   protected:
    QueryingSubscriberBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    QueryingSubscriberBase(::ze_owned_querying_subscriber_t* qs) : Owned(qs){};

   public:
    /// @name Methods

    /// @brief Make querying subscriber perform an additional query on a specified selector.
    /// The queried samples will be merged with the received publications and made available in the subscriber callback.
    /// @param key_expr the key expression matching resources to query.
    /// @param options query options.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void get(const KeyExpr& key_expr,
             zenoh::Session::GetOptions&& options = zenoh::Session::GetOptions::create_default(),
             ZResult* err = nullptr) const {
        ::z_get_options_t opts;
        z_get_options_default(&opts);
        opts.target = options.target;
        opts.consolidation = *interop::as_copyable_c_ptr(options.consolidation);
        opts.payload = zenoh::interop::as_moved_c_ptr(options.payload);
        opts.encoding = zenoh::interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = zenoh::interop::as_moved_c_ptr(options.source_info);
        opts.accept_replies = options.accept_replies;
        opts.allowed_destination = options.allowed_destination;
#endif
        opts.attachment = zenoh::interop::as_moved_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        ZResult res = ::ze_querying_subscriber_get(interop::as_loaned_c_ptr(*this),
                                                   zenoh::interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to perform get operation");
    }

    friend class SessionExt;
};

}  // namespace detail

template <class Handler>
class QueryingSubscriber;

/// @warning This API is deprecated. Please use zenoh::ext::AdvancedSubscriber.
/// @brief A Zenoh querying subscriber.
///
/// In addition to receiving the data it is subscribed to,
/// it also will fetch data from a Queryable at startup and peridodically (using  `QueryingSubscriber::get`).
/// @note Zenoh-c only.
template <>
class QueryingSubscriber<void> : public detail::QueryingSubscriberBase {
   protected:
    using QueryingSubscriberBase::QueryingSubscriberBase;
    friend class Session;
    friend struct zenoh::interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare querying subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_querying_subscriber(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Querying Subscriber");
    }
};

/// @warning This API is deprecated. Please use zenoh::ext::AdvancedSubscriber.
/// @brief A Zenoh querying subscriber declared by ``SessionExt::declare_querying_subscriber``.
///
/// In addition to receiving the data it is subscribed to,
/// it also will fetch data from a Queryable at startup and peridodically (using  ``QueryingSubscriber::get``).
/// @note Zenoh-c only.
/// @tparam Handler streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class QueryingSubscriber : public detail::QueryingSubscriberBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream querying subscriber from callback querying subscriber and handler.
    ///
    /// @param qs callback querying subscriber, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `s`. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    QueryingSubscriber(QueryingSubscriber<void>&& qs, Handler handler)
        : QueryingSubscriberBase(interop::as_owned_c_ptr(qs)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare querying subscriber, and return its handler, which can still be used to process any messages
    /// received prior to undeclaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_querying_subscriber(::z_move(this->_0)), err,
                             "Failed to undeclare Querying Subscriber");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to subscriber data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

}  // namespace zenoh::ext

namespace zenoh::interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of querying subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(zenoh::ext::QueryingSubscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::ext::detail::QueryingSubscriberBase&>(s)),
                          as_owned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of querying subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const zenoh::ext::QueryingSubscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::ext::detail::QueryingSubscriberBase&>(s)),
                          as_owned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of querying subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
[[deprecated]]
auto as_loaned_c_ptr(zenoh::ext::QueryingSubscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::ext::detail::QueryingSubscriberBase&>(s)),
                          as_loaned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of querying subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const zenoh::ext::QueryingSubscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::ext::detail::QueryingSubscriberBase&>(s)),
                          as_loaned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of querying subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(zenoh::ext::QueryingSubscriber<Handler>& s) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::ext::detail::QueryingSubscriberBase&>(s)),
                          as_moved_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of querying subscriber and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<zenoh::ext::QueryingSubscriber<Handler>>& s) -> decltype(as_moved_c_ptr(s.value())) {
    if (!s.has_value()) {
        return as_moved_c_ptr(s.value());
    } else {
        return {};
    }
}

/// @brief Move querying subscriber and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(zenoh::ext::QueryingSubscriber<Handler>&& s) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::ext::detail::QueryingSubscriberBase&>(s))),
                          move_to_c_obj(std::move(const_cast<Handler&>(s))));
}
}  // namespace zenoh::interop
#endif
