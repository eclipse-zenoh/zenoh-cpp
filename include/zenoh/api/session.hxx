
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

#include <optional>

#include "../detail/closures_concrete.hxx"
#include "base.hxx"
#include "closures.hxx"
#include "config.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "timestamp.hxx"
#if defined Z_FEATURE_UNSTABLE_API
#include "id.hxx"
#endif
#include "publisher.hxx"
#include "query_consolidation.hxx"
#include "subscriber.hxx"
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include "liveliness.hxx"
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
#include "shm/client_storage/client_storage.hxx"
#endif

#include <optional>

namespace zenoh {
/// A Zenoh session.
class Session : public Owned<::z_owned_session_t> {
    Session(zenoh::detail::null_object_t) : Owned(nullptr){};

   public:
    /// @brief Options to be passed when opening a ``Session``.
    struct SessionOptions {
        /// @name Fields
#ifdef ZENOHCXX_ZENOHPICO
        /// @brief For zenoh-pico only. If true, start background threads which handles the network
        /// traffic. If false, the threads should be called manually with ``Session::start_read_task`` and
        /// ``Session::start_lease_task`` or methods ``Session::read``, ``Session::send_keep_alive`` and
        /// ``Session::send_join`` should be called in loop.
        bool start_background_tasks = true;
#endif
        static SessionOptions create_default() { return {}; }
    };

    /// @brief Options to be passed when closing a ``Session``.
    struct SessionCloseOptions {
        /// @name Fields
        static SessionCloseOptions create_default() { return {}; }
    };

    /// @name Constructors

    /// @brief Create a new Session.
    /// @param config Zenoh session ``Config``.
    /// @param options Options to pass to session creation operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Session(Config&& config, SessionOptions&& options = SessionOptions::create_default(), ZResult* err = nullptr)
        : Owned(nullptr) {
        __ZENOH_RESULT_CHECK(::z_open(&this->_0, interop::as_moved_c_ptr(config), nullptr), err,
                             "Failed to open session");
#ifdef ZENOHCXX_ZENOHPICO
        if (err != nullptr && *err != Z_OK) return;
        if (options.start_background_tasks) {
            ZResult err_inner;
            this->start_read_task(&err_inner);
            if (err_inner == Z_OK) {
                this->start_lease_task(&err_inner);
            }
            if (err_inner == Z_OK) return;
            ::z_close(::z_move(this->_0), nullptr);
            __ZENOH_RESULT_CHECK(err_inner, err, "Failed to start background tasks");
        }
#else
        (void)options;
#endif
    }

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
    /// @brief Create a new Session with custom SHM client set.
    /// @param config Zenoh session ``Config``.
    /// @param shm_storage Storage with custom SHM clients.
    /// @param options Options to pass to session creation operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Session(Config&& config, const ShmClientStorage& shm_storage,
            SessionOptions&& options = SessionOptions::create_default(), ZResult* err = nullptr)
        : Owned(nullptr) {
        (void)options;
        __ZENOH_RESULT_CHECK(::z_open_with_custom_shm_clients(&this->_0, interop::as_moved_c_ptr(config),
                                                              interop::as_loaned_c_ptr(shm_storage)),
                             err, "Failed to open session");
    }
#endif

    /// @brief A factory method equivalent to a ``Session`` constructor.
    /// @param config Zenoh session ``Config``.
    /// @param options Options to pass to session creation operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return ``Session`` object. In case of failure it will be return in invalid state.
    static Session open(Config&& config, SessionOptions&& options = SessionOptions::create_default(),
                        ZResult* err = nullptr) {
        return Session(std::move(config), std::move(options), err);
    }

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API)
    /// @brief A factory method equivalent to a ``Session`` constructor for custom SHM clients list.
    /// @param config Zenoh session ``Config``.
    /// @param shm_storage Storage with custom SHM clients.
    /// @param options Options to pass to session creation operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return ``Session`` object. In case of failure it will be return in invalid state.
    static Session open(Config&& config, const ShmClientStorage& shm_storage,
                        SessionOptions&& options = SessionOptions::create_default(), ZResult* err = nullptr) {
        return Session(std::move(config), shm_storage, std::move(options), err);
    }
#endif

    /// @name Methods
#if defined Z_FEATURE_UNSTABLE_API
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the unique identifier of the zenoh node associated to this ``Session``.
    /// @return the unique identifier ``Id``.
    Id get_zid() const { return interop::into_copyable_cpp_obj<Id>(::z_info_zid(interop::as_loaned_c_ptr(*this))); }
#endif

    /// @brief Create ``KeyExpr`` instance with numeric id registered in ``Session`` routing tables (to reduce bandwith
    /// usage).
    /// @param key_expr ``KeyExpr`` to declare.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return Declared ``KeyExpr`` instance.
    KeyExpr declare_keyexpr(const KeyExpr& key_expr, ZResult* err = nullptr) const {
        KeyExpr k = interop::detail::null<KeyExpr>();
        __ZENOH_RESULT_CHECK(::z_declare_keyexpr(interop::as_owned_c_ptr(k), interop::as_loaned_c_ptr(*this),
                                                 interop::as_loaned_c_ptr(key_expr)),
                             err, std::string("Failed to declare key expression: ").append(k.as_string_view()));
        return k;
    }

    /// @brief Remove ``KeyExpr`` instance from ``Session`` routing table and drop ``KeyExpr`` instance.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @param key_expr ``KeyExpr`` instance to undeclare, that was previously returned by ``Session::declare_keyexpr``.
    void undeclare_keyexpr(KeyExpr&& key_expr, ZResult* err = nullptr) const {
        __ZENOH_RESULT_CHECK(::z_undeclare_keyexpr(interop::as_moved_c_ptr(key_expr), interop::as_loaned_c_ptr(*this)),
                             err, "Failed to undeclare key expression");
    }

    /// @brief Options passed to the ``get`` operation.
    struct GetOptions {
        /// @name Fields

        /// @brief The Queryables that should be target of the query.
        QueryTarget target = QueryTarget::Z_QUERY_TARGET_ALL;
        /// @brief The replies consolidation strategy to apply on replies to the query.
        QueryConsolidation consolidation = QueryConsolidation();
        /// @brief The priority of the get message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing get message.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief Whether Zenoh will NOT wait to batch get message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief An optional payload of the query.
        std::optional<Bytes> payload = {};
        /// @brief  An optional encoding of the query payload and/or attachment.
        std::optional<Encoding> encoding = {};
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info for the query.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief An optional attachment to the query.
        std::optional<Bytes> attachment = {};
        /// @brief The timeout for the query in milliseconds. 0 means default query timeout from zenoh configuration.
        uint64_t timeout_ms = 0;

        /// @name Methods
        /// @brief Create default option settings.
        static GetOptions create_default() { return {}; }
    };

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param key_expr ``KeyExpr`` the key expression matching resources to query.
    /// @param parameters the parameters string in URL format.
    /// @param on_reply callable that will be called once for each received reply.
    /// @param on_drop callable that will be called once all replies are received.
    /// @param options ``GetOptions`` query options.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void get(const KeyExpr& key_expr, const std::string& parameters, C&& on_reply, D&& on_drop,
             GetOptions&& options = GetOptions::create_default(), ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const Reply&>::value,
                      "on_reply should be callable with the following signature: void on_reply(zenoh::Reply& reply)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_reply_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Reply&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_reply), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_reply_call, detail::closures::_zenoh_on_drop, closure);
        ::z_get_options_t opts;
        z_get_options_default(&opts);
        opts.target = options.target;
        opts.consolidation = *interop::as_copyable_c_ptr(options.consolidation);
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;
        opts.payload = interop::as_moved_c_ptr(options.payload);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        __ZENOH_RESULT_CHECK(::z_get(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr),
                                     parameters.c_str(), ::z_move(c_closure), &opts),
                             err, "Failed to perform get operation");
    }

    /// @brief Query data from the matching queryables in the system. Replies are provided through a channel.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression matching resources to query.
    /// @param parameters the parameters string in URL format.
    /// @param channel channel instance.
    /// @param options query options.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return Reply handler.
    template <class Channel>
    typename Channel::template HandlerType<Reply> get(const KeyExpr& key_expr, const std::string& parameters,
                                                      Channel channel,
                                                      GetOptions&& options = GetOptions::create_default(),
                                                      ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Reply>();
        ::z_get_options_t opts;
        z_get_options_default(&opts);
        opts.target = options.target;
        opts.consolidation = *interop::as_copyable_c_ptr(options.consolidation);
        opts.payload = interop::as_moved_c_ptr(options.payload);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        ZResult res = ::z_get(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), parameters.c_str(),
                              ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to perform get operation");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return std::move(cb_handler_pair.second);
    }
    /// @brief Options to be passed to ``delete_resource`` operation
    struct DeleteOptions {
        /// @name Fields

        /// @brief The priority of the delete message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing delete message.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief Whether Zenoh will NOT wait to batch delete message with others to reduce the bandwith.
        bool is_express = false;
#if defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The delete operation reliability.
        Reliability reliability = Reliability::Z_RELIABILITY_BEST_EFFORT;
#endif
        /// @brief the timestamp of this message.
        std::optional<Timestamp> timestamp = {};

        /// @name Methods
        /// @brief Create default option settings.
        static DeleteOptions create_default() { return {}; }
    };

    /// @brief Undeclare a resource. Equivalent to ``Publisher::delete_resource``.
    /// @param key_expr  the key expression to delete the resource.
    /// @param options options to pass to delete operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void delete_resource(const KeyExpr& key_expr, DeleteOptions&& options = DeleteOptions::create_default(),
                         ZResult* err = nullptr) const {
        ::z_delete_options_t opts;
        z_delete_options_default(&opts);
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;
#if defined(Z_FEATURE_UNSTABLE_API)
        opts.reliability = options.reliability;
#endif

        __ZENOH_RESULT_CHECK(::z_delete(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), &opts),
                             err, "Failed to perform delete operation");
    }

    /// @brief Options passed to the ``Session::put`` operation.
    struct PutOptions {
        /// @name Fields

        /// @brief The priority of this message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing this message.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief Whether Zenoh will NOT wait to batch this message with others to reduce the bandwith.
        bool is_express = false;
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief Allowed destination.
        Locality allowed_destination = ::zc_locality_default();
#endif
        /// @brief the timestamp of this message.
        std::optional<Timestamp> timestamp = {};
        /// @brief  An optional encoding of the message payload and/or attachment.
        std::optional<Encoding> encoding = {};
#if defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The put operation reliability.
        Reliability reliability = Reliability::Z_RELIABILITY_BEST_EFFORT;
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info of this message.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief An optional attachment to the message.
        std::optional<Bytes> attachment = {};

        /// @name Methods
        /// @brief Create default option settings.
        static PutOptions create_default() { return {}; }
    };

    /// @brief Publish data to the matching subscribers in the system. Equivalent to ``Publisher::put``.
    /// @param key_expr the key expression to put the data.
    /// @param payload the data to publish.
    /// @param options options to pass to put operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void put(const KeyExpr& key_expr, Bytes&& payload, PutOptions&& options = PutOptions::create_default(),
             ZResult* err = nullptr) const {
        ::z_put_options_t opts;
        z_put_options_default(&opts);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;
#if defined(Z_FEATURE_UNSTABLE_API)
        opts.reliability = options.reliability;
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.allowed_destination = options.allowed_destination;
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);
        opts.timestamp = interop::as_copyable_c_ptr(options.timestamp);
        auto payload_ptr = interop::as_moved_c_ptr(payload);
        __ZENOH_RESULT_CHECK(
            ::z_put(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), payload_ptr, &opts), err,
            "Failed to perform put operation");
    }

    /// @brief Options to be passed when declaring a ``Queryable``
    struct QueryableOptions {
        /// @name Fields

        /// @brief The completeness of the Queryable.
        bool complete = false;

        /// @name Methods
        /// @brief Create default option settings.
        static QueryableOptions create_default() { return {}; }
    };

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests.
    /// @param key_expr the key expression to match the ``Session::get`` requests.
    /// @param on_query the callable to handle ``Query`` requests. Will be called once for each query.
    /// @param on_drop the drop callable. Will be called once, when ``Queryable`` is destroyed or undeclared.
    /// @param options options passed to queryable declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Queryable`` object.
    template <class C, class D>
    Queryable<void> declare_queryable(const KeyExpr& key_expr, C&& on_query, D&& on_drop,
                                      QueryableOptions&& options = QueryableOptions::create_default(),
                                      ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const Query&>::value,
                      "on_query should be callable with the following signature: void on_query(zenoh::Query& query)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_query_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Query&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_query), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_query_call, detail::closures::_zenoh_on_drop, closure);
        ::z_queryable_options_t opts;
        z_queryable_options_default(&opts);
        opts.complete = options.complete;

        Queryable<void> q(zenoh::detail::null_object);
        ZResult res = ::z_declare_queryable(interop::as_owned_c_ptr(q), interop::as_loaned_c_ptr(*this),
                                            interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Queryable");
        return q;
    }

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression to match the ``Session::get`` requests.
    /// @param channel an instance of channel.
    /// @param options options passed to queryable declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Queryable`` object.
    template <class Channel>
    Queryable<typename Channel::template HandlerType<Query>> declare_queryable(
        const KeyExpr& key_expr, Channel channel, QueryableOptions&& options = QueryableOptions::create_default(),
        ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Query>();
        ::z_queryable_options_t opts;
        z_queryable_options_default(&opts);
        opts.complete = options.complete;

        Queryable<void> q(zenoh::detail::null_object);
        ZResult res = ::z_declare_queryable(interop::as_owned_c_ptr(q), interop::as_loaned_c_ptr(*this),
                                            interop::as_loaned_c_ptr(key_expr), ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Queryable");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return Queryable<typename Channel::template HandlerType<Query>>(std::move(q),
                                                                        std::move(cb_handler_pair.second));
    }

    /// @brief Options to be passed when declaring a ``Subscriber``.
    struct SubscriberOptions {
        /// @name Fields

        /// @name Methods
        /// @brief Create default option settings.
        static SubscriberOptions create_default() { return {}; }
    };

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from.
    /// ``Session::put`` and ``Session::delete_resource`` requests.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression to match the publishers.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class C, class D>
    Subscriber<void> declare_subscriber(const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
                                        SubscriberOptions&& options = SubscriberOptions::create_default(),
                                        ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, const Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_sample_call, detail::closures::_zenoh_on_drop, closure);
        ::z_subscriber_options_t opts;
        z_subscriber_options_default(&opts);
        (void)options;
        Subscriber<void> s(zenoh::detail::null_object);
        ZResult res = ::z_declare_subscriber(interop::as_owned_c_ptr(s), interop::as_loaned_c_ptr(*this),
                                             interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Subscriber");
        return s;
    }

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from.
    /// ``Session::put`` and ``Session::delete_resource`` requests.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression to match the publishers.
    /// @param channel an instance of channel.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class Channel>
    Subscriber<typename Channel::template HandlerType<Sample>> declare_subscriber(
        const KeyExpr& key_expr, Channel channel, SubscriberOptions&& options = SubscriberOptions::create_default(),
        ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Sample>();
        ::z_subscriber_options_t opts;
        z_subscriber_options_default(&opts);
        (void)options;
        Subscriber<void> s(zenoh::detail::null_object);
        ZResult res =
            ::z_declare_subscriber(interop::as_owned_c_ptr(s), interop::as_loaned_c_ptr(*this),
                                   interop::as_loaned_c_ptr(key_expr), ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Subscriber");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return Subscriber<typename Channel::template HandlerType<Sample>>(std::move(s),
                                                                          std::move(cb_handler_pair.second));
    }

    /// @brief Options to be passed when declaring a ``Publisher``.
    struct PublisherOptions {
        /// @name Fields

        /// @brief The congestion control to apply when routing messages from this publisher.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief The priority of messages from this publisher.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief If true, Zenoh will not wait to batch this message with others to reduce the bandwith.
        bool is_express = false;
#if defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The publisher reliability.
        Reliability reliability = Reliability::Z_RELIABILITY_BEST_EFFORT;
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief Allowed destination.
        Locality allowed_destination = ::zc_locality_default();
#endif
        /// @brief Default encoding to use for Publisher::put.
        std::optional<Encoding> encoding = {};

        /// @name Methods
        /// @brief Create default option settings.
        static PublisherOptions create_default() { return {}; }
    };

    /// @brief Create a ``Publisher`` object to publish data to matching ``Subscriber`` objects.
    /// @param key_expr The key expression to match the subscribers.
    /// @param options Options passed to publisher declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Publisher`` object.
    Publisher declare_publisher(const KeyExpr& key_expr,
                                PublisherOptions&& options = PublisherOptions::create_default(),
                                ZResult* err = nullptr) const {
        ::z_publisher_options_t opts;
        z_publisher_options_default(&opts);
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;
#if defined(Z_FEATURE_UNSTABLE_API)
        opts.reliability = options.reliability;
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.allowed_destination = options.allowed_destination;
#endif
        opts.encoding = interop::as_moved_c_ptr(options.encoding);

        Publisher p = interop::detail::null<Publisher>();
        ZResult res = ::z_declare_publisher(interop::as_owned_c_ptr(p), interop::as_loaned_c_ptr(*this),
                                            interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Publisher");
        return p;
    }

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Fetches the Zenoh IDs of all connected routers.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a vector of all connected router Id.
    std::vector<Id> get_routers_z_id(ZResult* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const Id& z_id) { out.push_back(z_id); };
        typedef decltype(f) F;
        ::z_owned_closure_zid_t c_closure;
        using ClosureType = typename detail::closures::Closure<F, closures::None, void, const Id&>;
        auto closure = ClosureType::into_context(std::forward<F>(f), closures::none);
        ::z_closure(&c_closure, detail::closures::_zenoh_on_id_call, detail::closures::_zenoh_on_drop, closure);
        __ZENOH_RESULT_CHECK(::z_info_routers_zid(interop::as_loaned_c_ptr(*this), ::z_move(c_closure)), err,
                             "Failed to fetch router Ids");
        return out;
    }

    /// @brief Fetches the Zenoh IDs of all connected peers.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a vector of all connected peer Id.
    std::vector<Id> get_peers_z_id(ZResult* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const Id& z_id) { out.push_back(z_id); };
        typedef decltype(f) F;
        ::z_owned_closure_zid_t c_closure;
        auto closure = detail::closures::Closure<F, closures::None, void, const Id&>::into_context(std::forward<F>(f),
                                                                                                   closures::none);
        ::z_closure(&c_closure, detail::closures::_zenoh_on_id_call, detail::closures::_zenoh_on_drop, closure);
        __ZENOH_RESULT_CHECK(::z_info_peers_zid(interop::as_loaned_c_ptr(*this), ::z_move(c_closure)), err,
                             "Failed to fetch peer Ids");
        return out;
    }
#endif

#ifdef ZENOHCXX_ZENOHPICO
    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note zenoh-pico only
    void start_read_task(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_start_read_task(interop::as_loaned_c_ptr(*this), nullptr), err,
                             "Failed to start read task");
    }

    /// @brief Stop the read task.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note zenoh-pico only
    void stop_read_task(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_stop_read_task(interop::as_loaned_c_ptr(*this)), err, "Failed to stop read task");
    }

    /// @brief Start a separate task to handle the session lease.  This task will send KeepAlive messages when needed
    /// and will close the session when the lease is expired. When operating over a multicast transport, it also
    /// periodically sends the Join messages.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note zenoh-pico only
    void start_lease_task(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_start_lease_task(interop::as_loaned_c_ptr(*this), NULL), err,
                             "Failed to start lease task");
    }

    /// @brief Stop the lease task.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note zenoh-pico only
    void stop_lease_task(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_stop_lease_task(interop::as_loaned_c_ptr(*this)), err, "Failed to stop lease task");
    }

    /// @brief Triggers a single execution of reading procedure from the network and processes of any received the
    /// message.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note zenoh-pico only
    void read(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_read(interop::as_loaned_c_ptr(*this), nullptr), err, "Failed to perform read");
    }

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void send_keep_alive(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_send_keep_alive(interop::as_loaned_c_ptr(*this), nullptr), err,
                             "Failed to perform send_keep_alive");
    }

    /// @brief Triggers a single execution of join procedure: send the Join message.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void send_join(ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_send_join(interop::as_loaned_c_ptr(*this), nullptr), err,
                             "Failed to perform send_join");
    }
#endif

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Options to pass to ``Session::liveliness_declare_token``.
    struct LivelinessDeclarationOptions {
       protected:
        uint8_t _dummy = 0;

       public:
        static LivelinessDeclarationOptions create_default() { return {}; }
    };

    /// Declares a liveliness token on the network.
    ///
    /// Liveliness token subscribers on an intersecting key expression will receive a PUT sample when connectivity
    /// is achieved, and a DELETE sample if it's lost.
    ///
    /// @param key_expr: A keyexpr to declare a liveliess token for.
    /// @param options: Liveliness token declaration properties.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``LivelinessToken``.
    LivelinessToken liveliness_declare_token(
        const KeyExpr& key_expr,
        LivelinessDeclarationOptions&& options = LivelinessDeclarationOptions::create_default(),
        ZResult* err = nullptr) {
        LivelinessToken t = interop::detail::null<LivelinessToken>();
        ::zc_liveliness_declaration_options_t opts;
        zc_liveliness_declaration_options_default(&opts);
        (void)options;
        __ZENOH_RESULT_CHECK(::zc_liveliness_declare_token(interop::as_owned_c_ptr(t), interop::as_loaned_c_ptr(*this),
                                                           interop::as_loaned_c_ptr(key_expr), &opts),
                             err, "Failed to perform liveliness_declare_token operation");
        return t;
    }

    /// @brief Options to pass to ``Session::liveliness_declare_subscriber``.
    struct LivelinessSubscriberOptions {
       public:
        bool history = false;

        static LivelinessSubscriberOptions create_default() { return {}; }
    };

    /// @brief Declares a subscriber on liveliness tokens that intersect `key_expr`.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr  the key expression to subscribe to.
    /// @param on_sample the callabl that will be called each time a liveliness token status is changed.
    /// @param on_drop the callable that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class C, class D>
    Subscriber<void> liveliness_declare_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        LivelinessSubscriberOptions&& options = LivelinessSubscriberOptions::create_default(),
        ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, const Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_sample_call, detail::closures::_zenoh_on_drop, closure);
        ::zc_liveliness_subscriber_options_t opts;
        zc_liveliness_subscriber_options_default(&opts);
        opts.history = options.history;
        Subscriber<void> s(zenoh::detail::null_object);
        ZResult res =
            ::zc_liveliness_declare_subscriber(interop::as_owned_c_ptr(s), interop::as_loaned_c_ptr(*this),
                                               interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Liveliness Token Subscriber");
        return s;
    }

    /// @brief Declare a subscriber on liveliness tokens that intersect `key_expr`.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression to subscribe to.
    /// @param channel an instance of channel.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class Channel>
    Subscriber<typename Channel::template HandlerType<Sample>> liveliness_declare_subscriber(
        const KeyExpr& key_expr, Channel channel,
        LivelinessSubscriberOptions&& options = LivelinessSubscriberOptions::create_default(),
        ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Sample>();
        ::zc_liveliness_subscriber_options_t opts;
        zc_liveliness_subscriber_options_default(&opts);
        opts.history = options.history;
        Subscriber<void> s(zenoh::detail::null_object);
        ZResult res = ::zc_liveliness_declare_subscriber(interop::as_owned_c_ptr(s), interop::as_loaned_c_ptr(*this),
                                                         interop::as_loaned_c_ptr(key_expr),
                                                         ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Liveliness Token Subscriber");
        if (res != Z_OK) ::z_drop(::z_move(*interop::as_moved_c_ptr(cb_handler_pair.second)));
        return Subscriber<typename Channel::template HandlerType<Sample>>(std::move(s),
                                                                          std::move(cb_handler_pair.second));
    }

    /// @brief Options to pass to ``Session::liveliness_get``.
    struct LivelinessGetOptions {
        /// @name Fields

        /// @brief The timeout for the query in milliseconds.
        uint32_t timeout_ms = 10000;

        /// @name Methods
        /// @brief Create default option settings.
        static LivelinessGetOptions create_default() { return {}; }
    };

    /// @brief Queries liveliness tokens currently on the network with a key expression intersecting with `key_expr`.
    ///
    /// @param session: The Zenoh session.
    /// @param key_expr: The key expression to query liveliness tokens for.
    /// @param callback: The callable that will be called for each received reply.
    /// @param options: Additional options for the liveliness get operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void liveliness_get(const KeyExpr& key_expr, C&& on_reply, D&& on_drop,
                        LivelinessGetOptions&& options = LivelinessGetOptions::create_default(),
                        ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const Reply&>::value,
                      "on_reply should be callable with the following signature: void on_reply(zenoh::Reply& reply)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_reply_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Reply&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_reply), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_reply_call, detail::closures::_zenoh_on_drop, closure);
        ::zc_liveliness_get_options_t opts;
        zc_liveliness_get_options_default(&opts);
        opts.timeout_ms = options.timeout_ms;

        __ZENOH_RESULT_CHECK(::zc_liveliness_get(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr),
                                                 ::z_move(c_closure), &opts),
                             err, "Failed to perform liveliness_get operation");
    }

    /// @brief Queries liveliness tokens currently on the network with a key expression intersecting with `key_expr`.
    /// @tparam Channel the type of channel used to create stream of data (see `zenoh::channels::FifoChannel` or
    /// `zenoh::channels::RingChannel`).
    /// @param key_expr the key expression to query liveliness tokens for.
    /// @param channel channel instance.
    /// @param options  additional options for the liveliness get operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return Reply handler.
    template <class Channel>
    typename Channel::template HandlerType<Reply> liveliness_get(
        const KeyExpr& key_expr, Channel channel,
        LivelinessGetOptions&& options = LivelinessGetOptions::create_default(), ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Reply>();
        ::zc_liveliness_get_options_t opts;
        zc_liveliness_get_options_default(&opts);
        opts.timeout_ms = options.timeout_ms;

        ZResult res = ::zc_liveliness_get(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr),
                                          ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to perform liveliness_get operation");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return std::move(cb_handler_pair.second);
    }

#endif
    /// @brief Create Timestamp from session id.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Timestamp new_timestamp(ZResult* err = nullptr) {
        ::z_timestamp_t t;
        __ZENOH_RESULT_CHECK(z_timestamp_new(&t, interop::as_loaned_c_ptr(*this)), err, "Failed to create timestamp");
        return interop::into_copyable_cpp_obj<Timestamp>(t);
    }

    /// @brief Close and invalidate the session. This also undeclares all non-undeclared Subscriber and Queryable
    /// callbacks.
    /// @param options options to pass to close operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void close(SessionCloseOptions&& options = SessionCloseOptions::create_default(), ZResult* err = nullptr) && {
        (void)options;
        __ZENOH_RESULT_CHECK(::z_close(interop::as_moved_c_ptr(*this), nullptr), err, "Failed to close the session");
    }
};
}  // namespace zenoh