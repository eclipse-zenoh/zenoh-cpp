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
#include "../session.hxx"
#include "../source_info.hxx"
#include "../subscriber.hxx"
#include "miss.hxx"

namespace zenoh::ext {

namespace detail {
class AdvancedSubscriberBase : public Owned<::ze_owned_advanced_subscriber_t> {
   protected:
    AdvancedSubscriberBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    AdvancedSubscriberBase(::ze_owned_advanced_subscriber_t* s) : Owned(s){};

   public:
    /// @name Methods

    /// @brief Construct sample miss listener, registering a callback for notifying about missed samples.
    ///
    /// @param on_miss_detected: the callable that will be called every time when missed samples are detected.
    /// @param on_drop the callable that will be called once sample miss listener is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``SampleMissListener`` object.
    /// @note Zenoh-c only.
    template <class C, class D>
    [[nodiscard]] SampleMissListener<void> declare_sample_miss_listener(C&& on_miss_detected, D&& on_drop,
                                                                        zenoh::ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const Miss&>::value,
                      "on_miss_detected should be callable with the following signature: void on_status_change(const "
                      "zenoh::ext::Miss& miss)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::ze_owned_closure_miss_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const Miss&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_miss_detected), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_miss_detected_call,
                    zenoh::detail::closures::_zenoh_on_drop, closure);
        auto m = zenoh::interop::detail::null<SampleMissListener<void>>();
        ZResult res = ::ze_advanced_subscriber_declare_sample_miss_listener(
            zenoh::interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(m), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Sample Miss Listener");
        return m;
    }

    /// @brief Declare sample miss listener, registering a callback for notifying about detected missed samples.
    /// The callback will be run in the background until the corresponding advanced subscriber is destroyed.
    ///
    /// @param on_miss_detected: the callable that will be called every time when missed samples are detected.
    /// @param on_drop the callable that will be called once sample miss listener is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note Zenoh-c only.
    template <class C, class D>
    void declare_background_sample_miss_listener(C&& on_miss_detected, D&& on_drop,
                                                 zenoh::ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const Miss&>::value,
                      "on_miss_detected should be callable with the following signature: void on_status_change(const "
                      "zenoh::ext::Miss& miss)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::ze_owned_closure_miss_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const Miss&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_miss_detected), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_miss_detected_call,
                    zenoh::detail::closures::_zenoh_on_drop, closure);

        ZResult res = ::ze_advanced_subscriber_declare_background_sample_miss_listener(
            zenoh::interop::as_loaned_c_ptr(*this), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare background Sample Miss Listener");
    }

    /// @brief Declares a liveliness token listener for matching publishers detection. Only advanced publishers,
    /// enabling publisher detection can be detected.
    /// @param on_sample the callable that will be called each time a liveliness token status is changed.
    /// @param on_drop the callable that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to listener declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``zenoh::Subscriber`` object.
    template <class C, class D>
    [[nodiscard]] zenoh::Subscriber<void> detect_publishers(
        C&& on_sample, D&& on_drop,
        zenoh::Session::LivelinessSubscriberOptions&& options =
            zenoh::Session::LivelinessSubscriberOptions::create_default(),
        ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, zenoh::Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, zenoh::Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        ::z_liveliness_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        zenoh::Subscriber<void> s = zenoh::interop::detail::null<zenoh::Subscriber<void>>();
        zenoh::ZResult res = ::ze_advanced_subscriber_detect_publishers(
            zenoh::interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(s), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Liveliness Token Subscriber");
        return s;
    }

    /// @brief Declares a background liveliness token listener for matching publishers detection. Only advanced
    /// publishers, enabling publisher detection can be detected. The subscriber callback will be run in the background
    /// until the corresponding session is closed or destroyed.
    /// @param on_sample the callable that will be called each time a liveliness token status is changed.
    /// @param on_drop the callable that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note Zenoh-c only.
    template <class C, class D>
    void detect_publishers_background(C&& on_sample, D&& on_drop,
                                      zenoh::Session::LivelinessSubscriberOptions&& options =
                                          zenoh::Session::LivelinessSubscriberOptions::create_default(),
                                      zenoh::ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, zenoh::Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, zenoh::Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        ::z_liveliness_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        zenoh::ZResult res = ::ze_advanced_subscriber_detect_publishers_background(
            zenoh::interop::as_loaned_c_ptr(*this), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Liveliness Token Subscriber");
    }

    /// @brief  Declares a liveliness token listener for matching publishers detection. Only advanced publishers,
    /// enabling publisher detection can be detected.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param channel an instance of channel.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class Channel>
    [[nodiscard]] zenoh::Subscriber<typename Channel::template HandlerType<Sample>> detect_publishers(
        Channel channel,
        zenoh::Session::LivelinessSubscriberOptions&& options =
            zenoh::Session::LivelinessSubscriberOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<zenoh::Sample>();
        ::z_liveliness_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        zenoh::Subscriber<void> s = zenoh::interop::detail::null<zenoh::Subscriber<void>>();
        zenoh::ZResult res =
            ::z_liveliness_declare_subscriber(zenoh::interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(s),
                                              ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Liveliness Token Subscriber");
        if (res != Z_OK) ::z_drop(::z_move(*zenoh::interop::as_moved_c_ptr(cb_handler_pair.second)));
        return zenoh::Subscriber<typename Channel::template HandlerType<zenoh::Sample>>(
            std::move(s), std::move(cb_handler_pair.second));
    }

    /// @brief Get the key expression of the advanced subscriber.
    const KeyExpr& get_keyexpr() const {
        return zenoh::interop::as_owned_cpp_ref<zenoh::KeyExpr>(
            ::ze_advanced_subscriber_keyexpr(zenoh::interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Get the id of the advanced subscriber.
    /// @return id of this advanced subscriber.
    EntityGlobalId get_id() const {
        return zenoh::interop::into_copyable_cpp_obj<zenoh::EntityGlobalId>(
            ::ze_advanced_subscriber_id(zenoh::interop::as_loaned_c_ptr(*this)));
    }
};
}  // namespace detail

template <class Handler>
class AdvancedSubscriber;

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A Zenoh advanced subscriber.
///
/// In addition to receiving the data it is subscribed to,
/// it also will fetch data from a Queryable at startup and peridodically (using  `AdvancedSubscriber::get`).
/// @note Zenoh-c only.
template <>
class AdvancedSubscriber<void> : public detail::AdvancedSubscriberBase {
   protected:
    using AdvancedSubscriberBase::AdvancedSubscriberBase;
    friend struct zenoh::interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare advanced subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_advanced_subscriber(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Advanced Subscriber");
    }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An owned Zenoh advanced subscriber.
///
/// In addition to receiving the data it is subscribed to,
/// it is also able to receive notifications regarding missed samples and/or automatically recover them.
/// @note Zenoh-c only.
/// @tparam Handler streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class AdvancedSubscriber : public detail::AdvancedSubscriberBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream advanced subscriber from callback advanced subscriber and handler.
    ///
    /// @param s callback advanced subscriber, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `s`. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    AdvancedSubscriber(AdvancedSubscriber<void>&& s, Handler handler)
        : AdvancedSubscriberBase(interop::as_owned_c_ptr(s)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare advanced subscriber, and return its handler, which can still be used to process any messages
    /// received prior to undeclaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_advanced_subscriber(::z_move(this->_0)), err,
                             "Failed to undeclare Querying Subscriber");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to advanced subscriber data stream.
    const Handler& handler() const { return _handler; };
};

}  // namespace zenoh::ext

namespace zenoh::interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of advanced subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(zenoh::ext::AdvancedSubscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::ext::detail::AdvancedSubscriberBase&>(s)),
                          as_owned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of advanced subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const zenoh::ext::AdvancedSubscriber<Handler>& s) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::ext::detail::AdvancedSubscriberBase&>(s)),
                          as_owned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of advanced subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(zenoh::ext::AdvancedSubscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::ext::detail::AdvancedSubscriberBase&>(s)),
                          as_loaned_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of advanced subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const zenoh::ext::AdvancedSubscriber<Handler>& s) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::ext::detail::AdvancedSubscriberBase&>(s)),
                          as_loaned_c_ptr(s.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of advanced subscriber and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(zenoh::ext::AdvancedSubscriber<Handler>& s) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::ext::detail::AdvancedSubscriberBase&>(s)),
                          as_moved_c_ptr(const_cast<Handler&>(s.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of advanced subscriber and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<zenoh::ext::AdvancedSubscriber<Handler>>& s) -> decltype(as_moved_c_ptr(s.value())) {
    if (!s.has_value()) {
        return as_moved_c_ptr(s.value());
    } else {
        return {};
    }
}

/// @brief Move advanced subscriber and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(zenoh::ext::AdvancedSubscriber<Handler>&& s) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::ext::detail::AdvancedSubscriberBase&>(s))),
                          move_to_c_obj(std::move(const_cast<Handler&>(s))));
}
}  // namespace zenoh::interop
#endif
