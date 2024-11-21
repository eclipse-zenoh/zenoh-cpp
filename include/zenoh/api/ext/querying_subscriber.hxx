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
#include "../base.hxx"
#include "../interop.hxx"
#include "../keyexpr.hxx"
#include "../session.hxx"

namespace zenoh {
namespace ext {

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
        opts.payload = interop::as_moved_c_ptr(options.payload);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
        opts.accept_replies = options.accept_replies;
        opts.allowed_destination = options.allowed_destination;
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        ZResult res =
            ::ze_querying_subscriber_get(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to perform get operation");
    }

    friend class zenoh::Session;
};

}  // namespace detail

template <class Handler>
class QueryingSubscriber;

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
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
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_querying_subscriber(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Querying Subscriber");
    }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A Zenoh querying subscriber declared by ``zenoh::Session::declare_querying_subscriber``.
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
        __ZENOH_RESULT_CHECK(::ze_undeclare_querying_subscriber(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Querying Subscriber");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to subscriber data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

}  // namespace ext

namespace interop {
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
}  // namespace interop

template <class C, class D>
[[nodiscard]] ext::QueryingSubscriber<void> Session::declare_querying_subscriber(const KeyExpr& key_expr, C&& on_sample,
                                                                                 D&& on_drop,
                                                                                 QueryingSubscriberOptions&& options,
                                                                                 ZResult* err) const {
    static_assert(std::is_invocable_r<void, C, const Sample&>::value,
                  "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
    static_assert(std::is_invocable_r<void, D>::value,
                  "on_drop should be callable with the following signature: void on_drop()");
    ::z_owned_closure_sample_t c_closure;
    using Cval = std::remove_reference_t<C>;
    using Dval = std::remove_reference_t<D>;
    using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Sample&>;
    auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
    ::z_closure(&c_closure, detail::closures::_zenoh_on_sample_call, detail::closures::_zenoh_on_drop, closure);
    ::ze_querying_subscriber_options_t opts;
    ze_querying_subscriber_options_default(&opts);
    opts.query_selector = interop::as_loaned_c_ptr(options.query_keyexpr);
#if defined(Z_FEATURE_UNSTABLE_API)
    opts.allowed_origin = options.allowed_origin;
    opts.query_accept_replies = options.query_accept_replies;
#endif
    opts.query_target = options.query_target;
    opts.query_consolidation = *interop::as_copyable_c_ptr(options.query_consolidation);
    opts.query_timeout_ms = options.query_timeout_ms;
    ext::QueryingSubscriber<void> qs = interop::detail::null<ext::QueryingSubscriber<void>>();
    ZResult res = ::ze_declare_querying_subscriber(interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(qs),
                                                   interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
    __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Querying Subscriber");
    return qs;
}

template <class C, class D>
void Session::declare_background_querying_subscriber(const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
                                                     QueryingSubscriberOptions&& options, ZResult* err) const {
    static_assert(std::is_invocable_r<void, C, const Sample&>::value,
                  "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
    static_assert(std::is_invocable_r<void, D>::value,
                  "on_drop should be callable with the following signature: void on_drop()");
    ::z_owned_closure_sample_t c_closure;
    using Cval = std::remove_reference_t<C>;
    using Dval = std::remove_reference_t<D>;
    using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Sample&>;
    auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
    ::z_closure(&c_closure, detail::closures::_zenoh_on_sample_call, detail::closures::_zenoh_on_drop, closure);
    ::ze_querying_subscriber_options_t opts;
    ze_querying_subscriber_options_default(&opts);
    opts.query_selector = interop::as_loaned_c_ptr(options.query_keyexpr);
#if defined(Z_FEATURE_UNSTABLE_API)
    opts.allowed_origin = options.allowed_origin;
    opts.query_accept_replies = options.query_accept_replies;
#endif
    opts.query_target = options.query_target;
    opts.query_consolidation = *interop::as_copyable_c_ptr(options.query_consolidation);
    ;
    opts.query_timeout_ms = options.query_timeout_ms;
    ZResult res = ::ze_declare_background_querying_subscriber(
        interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
    __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Querying Subscriber");
}

template <class Channel>
[[nodiscard]] ext::QueryingSubscriber<typename Channel::template HandlerType<Sample>>
Session::declare_querying_subscriber(const KeyExpr& key_expr, Channel channel, QueryingSubscriberOptions&& options,
                                     ZResult* err) const {
    auto cb_handler_pair = channel.template into_cb_handler_pair<Sample>();
    ::ze_querying_subscriber_options_t opts;
    ze_querying_subscriber_options_default(&opts);
    opts.query_selector = interop::as_loaned_c_ptr(options.query_keyexpr);
#if defined(Z_FEATURE_UNSTABLE_API)
    opts.allowed_origin = options.allowed_origin;
    opts.query_accept_replies = options.query_accept_replies;
#endif
    opts.query_target = options.query_target;
    opts.query_consolidation = *interop::as_copyable_c_ptr(options.query_consolidation);
    opts.query_timeout_ms = options.query_timeout_ms;
    ext::QueryingSubscriber<void> qs = interop::detail::null<ext::QueryingSubscriber<void>>();
    ZResult res =
        ::ze_declare_querying_subscriber(interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(qs),
                                         interop::as_loaned_c_ptr(key_expr), ::z_move(cb_handler_pair.first), &opts);
    __ZENOH_RESULT_CHECK(res, err, "Failed to declare Querying Subscriber");
    if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
    return ext::QueryingSubscriber<typename Channel::template HandlerType<Sample>>(std::move(qs),
                                                                                   std::move(cb_handler_pair.second));
}

}  // namespace zenoh
#endif
