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

#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1)

#include "../detail/closures_concrete.hxx"
#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"
#include "reply.hxx"
#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
#include "matching.hxx"
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif
#include <optional>

namespace zenoh {
class Session;

/// A Zenoh Querier. Constructed by ``Session::declare_querier`` method. Queriers allow to send queries to a queryable.
class Querier : public Owned<::z_owned_querier_t> {
    Querier(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @brief Options passed to the ``Querier::get`` operation.
    struct GetOptions {
        /// @name Fields

        /// @brief An optional payload of the query.
        std::optional<Bytes> payload = {};
        /// @brief  An optional encoding of the query payload and/or attachment.
        std::optional<Encoding> encoding = {};
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info for the query.
        /// @note Zenoh-c only.
        std::optional<SourceInfo> source_info = {};
#endif

        /// @brief An optional attachment to the query.
        std::optional<Bytes> attachment = {};

        /// @name Methods

        /// @brief Create default option settings.
        static GetOptions create_default() { return {}; }
    };

    /// @name Methods

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param parameters the parameters string in URL format.
    /// @param options Options to pass to get operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void get(const std::string& parameters, C&& on_reply, D&& on_drop,
             GetOptions&& options = GetOptions::create_default(), ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, Reply&>::value,
                      "on_reply should be callable with the following signature: void on_reply(zenoh::Reply& reply)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_reply_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, Reply&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_reply), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_reply_call, detail::closures::_zenoh_on_drop, closure);
        ::z_querier_get_options_t opts;
        z_querier_get_options_default(&opts);
        opts.payload = interop::as_moved_c_ptr(options.payload);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);

        __ZENOH_RESULT_CHECK(
            ::z_querier_get(interop::as_loaned_c_ptr(*this), parameters.c_str(), ::z_move(c_closure), &opts), err,
            "Failed to perform Querier::get operation");
    }

    /// @brief Query data from the matching queryables in the system. Replies are provided through a channel.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param parameters the parameters string in URL format.
    /// @param channel channel instance.
    /// @param options Options to pass to get operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return reply handler.
    template <class Channel>
    typename Channel::template HandlerType<Reply> get(const std::string& parameters, Channel channel,
                                                      GetOptions&& options = GetOptions::create_default(),
                                                      ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Reply>();
        ::z_querier_get_options_t opts;
        z_querier_get_options_default(&opts);
        opts.payload = interop::as_moved_c_ptr(options.payload);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);

        ZResult res = ::z_querier_get(interop::as_loaned_c_ptr(*this), parameters.c_str(),
                                      ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to perform Querier::get operation");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return std::move(cb_handler_pair.second);
    }

    /// @brief Get the key expression of the querier.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_querier_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Undeclares querier.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_querier(interop::as_moved_c_ptr(*this)), err, "Failed to undeclare querier");
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the querier.
    /// @return id of this querier.
    EntityGlobalId get_id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(::z_querier_id(interop::as_loaned_c_ptr(*this)));
    }
#endif

#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
    /// @brief Construct matching listener, registering a callback for notifying queryables matching with a given
    /// querier's key expression and target.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the querier
    /// changes (i.e. if last querier disconnects or when the first querier connects).
    /// @param on_drop the callable that will be called once matching listener is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``MatchingListener`` object.
    template <class C, class D>
    [[nodiscard]] MatchingListener<void> declare_matching_listener(C&& on_status_change, D&& on_drop,
                                                                   ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_status_change_call, detail::closures::_zenoh_on_drop,
                    closure);
        MatchingListener<void> m(zenoh::detail::null_object);
        ZResult res = ::z_querier_declare_matching_listener(interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(m),
                                                            ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Matching Listener");
        return m;
    }

    /// @brief Construct matching listener, delivering notification on querier status change through a streaming
    /// handler.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param channel: an instance of channel.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``MatchingListener`` object.
    template <class Channel>
    [[nodiscard]] MatchingListener<typename Channel::template HandlerType<MatchingStatus>> declare_matching_listener(
        Channel channel, ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Query>();
        MatchingListener<void> m(zenoh::detail::null_object);
        ZResult res = ::z_querier_declare_matching_listener(interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(m),
                                                            ::z_move(cb_handler_pair.first));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Matching Listener");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return MatchingListener<typename Channel::template HandlerType<MatchingStatus>>(
            std::move(m), std::move(cb_handler_pair.second));
    }

    /// @brief Declare matching listener, registering a callback for notifying queryables matching with a given
    /// querier. The callback will be run in the background until the corresponding querier is destroyed.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the querier
    /// changes (i.e. if last queryable disconnects or when the first queryable connects).
    /// @param on_drop the callable that will be called once querier is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void declare_background_matching_listener(C&& on_status_change, D&& on_drop, ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_status_change_call, detail::closures::_zenoh_on_drop,
                    closure);
        ZResult res =
            ::z_querier_declare_background_matching_listener(interop::as_loaned_c_ptr(*this), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare background Matching Listener");
    }

    /// @brief Gets querier matching status - i.e. if there are any queryables matching its key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    MatchingStatus get_matching_status(ZResult* err = nullptr) const {
        ::z_matching_status_t m;
        ZResult res = ::z_querier_get_matching_status(interop::as_loaned_c_ptr(*this), &m);
        __ZENOH_RESULT_CHECK(res, err, "Failed to get matching status");
        return {m.matching};
    }
#endif
};

}  // namespace zenoh
#endif
