
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

#include "base.hxx"
#include "../detail/interop.hxx"
#include "../detail/closures_concrete.hxx"

#include "enums.hxx"
#include "config.hxx"
#include "id.hxx"
#include "publisher.hxx"
#include "subscriber.hxx"
#include "queryable.hxx"
#include "query_consolidation.hxx"
#include "closures.hxx"

#include <optional>



namespace zenoh {
enum class FifoChannelType {
    Blocking,
    NonBlocking
};


template<FifoChannelType C>
class ReplyFifoChannel: public Owned<::z_owned_reply_channel_closure_t> {
private:
    bool active = true;
public:
    using Owned::Owned;
    /// @brief Fetches next available reply.
    /// @return Reply. Might return invalid reply in case of Non-blocking channel if there are no replies available.
    /// In this case is is possible to verify whether the channel is still active (and might receive more replies in the future)
    /// by calling  ``ReplyFifoChannel::is_active()``.
    Reply get_next_reply() {
        Reply r(nullptr);
        bool res = ::z_call(this->loan(), detail::as_owned_c_ptr(r));
        if constexpr (C == FifoChannelType::NonBlocking) {
            active = static_cast<bool>(r) || !res;
        } else {
            active = static_cast<bool>(r);
        }
        return r;
    }

    /// @brief Verifies if channel is still active.
    /// @return True if channel is still active (i. e. might receive more valid replies in the future). Inactive channel will
    /// always return an invalid ``Reply`` when calling  ``ReplyFifoChannel::next_reply()`.
    bool is_active() const { return active; }
};

/// A Zenoh session.
class Session : public Owned<::z_owned_session_t> {
public:
    using Owned::Owned;

    Session(Config&& config, ZError* err = nullptr) : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(
            ::z_open(&this->_0, detail::as_owned_c_ptr(config)),
            err,
            "Failed to open session"
        );
    }

    /// @name Methods

#ifdef ZENOHCXX_ZENOHC
    /// @brief Create a shallow copy of the session
    /// @return a new ``Session`` instance
    /// @note zenoh-c only
    Session clone() const {
        Session s(nullptr);
        ::zc_session_clone(this->loan(), &s._0);
        return s;
    }
#endif

    /// @brief Get the unique identifier of the zenoh node associated to this ``Session``
    /// @return the unique identifier ``Id``
    Id get_zid() const { 
         return Id(::z_info_zid(this->loan())); 
    }

    /// @brief Create ``KeyExpr`` instance with numeric id registered in ``Session`` routing tables
    /// @param key_expr ``KeyExpr`` to declare
    /// @return Declared ``KeyExpr`` instance
    KeyExpr declare_keyexpr(const KeyExpr& key_expr, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_declare_keyexpr(detail::as_owned_c_ptr(k), this->loan(), detail::loan(key_expr)),
            err,
            std::string("Failed to declare key expression: ").append(k.as_string_view())
        );
        return k;
    }

    /// @brief Remove ``KeyExpr`` instance from ``Session`` and drop ``KeyExpr`` instance
    /// @param keyexpr ``KeyExpr`` instance to drop
    void undeclare_keyexpr(KeyExpr&& key_expr, ZError* err = nullptr) const {
        __ZENOH_ERROR_CHECK(
            ::z_undeclare_keyexpr(detail::as_owned_c_ptr(key_expr), this->loan()),
            err,
            "Failed to undeclare key expression"
        );
    }

    /// @brief Options passed to the ``get()`` operation
    struct GetOptions {
        /// @brief The Queryables that should be target of the query.
        QueryTarget target = QueryTarget::Z_QUERY_TARGET_ALL;
        /// @brief The replies consolidation strategy to apply on replies to the query.
        QueryConsolidation consolidation = QueryConsolidation();
        /// @brief An optional payload of the query.
        std::optional<Bytes> payload = {};
        /// @brief  An optional encoding of the query payload and/or attachment.
        std::optional<Encoding> encoding = {};
        /// @brief An optional attachment to the query.
        std::optional<Bytes> attachment = {};
        /// @brief The timeout for the query in milliseconds. 0 means default query timeout from zenoh configuration.
        uint64_t timeout_ms = 0;

        /// @brief Returns default option settings
        static GetOptions create_default() { return {}; }
    };

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param key_expr ``KeyExpr`` the key expression matching resources to query
    /// @param parameters the parameters string in URL format
    /// @param on_reply callback that will be called once for each received reply
    /// @param on_drop callback that will be called once all replies are received
    /// @param options ``GetOptions`` query options
    template<class C, class D>
    void get(
        const KeyExpr& key_expr, const std::string& parameters, C&& on_reply, D&& on_drop, 
        GetOptions&& options = GetOptions::create_default(), ZError* err = nullptr
    ) const {
        static_assert(
            std::is_invocable_r<void, C, const Reply&>::value,
            "on_reply should be callable with the following signature: void on_reply(const zenoh::Reply& reply)"
        );
        static_assert(
            std::is_invocable_r<void, D>::value,
            "on_drop should be callable with the following signature: void on_drop()"
        );
        ::z_owned_closure_reply_t c_closure;
        using ClosureType = typename detail::closures::Closure<C, D, void, const Reply&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_reply), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_reply_call, detail::closures::_zenoh_on_drop, closure);
        ::z_get_options_t opts;
        opts.target = options.target;
        opts.consolidation = static_cast<const z_query_consolidation_t&>(options.consolidation);
        opts.payload = detail::as_owned_c_ptr(options.payload);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.attachment = detail::as_owned_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        __ZENOH_ERROR_CHECK(
            ::z_get(this->loan(), detail::loan(key_expr), parameters.c_str(), ::z_move(c_closure), &opts),
            err,
            "Failed to perform get operation"
        );
    }

    /// @brief Query data from the matching queryables in the system. Replies are provided through a channel.
    /// @param key_expr ``KeyExpr`` the key expression matching resources to query
    /// @param parameters the parameters string in URL format
    /// @param bound Capacity of the channel, if different from 0, the channel will be bound and apply back-pressure when full
    /// @param options ``GetOptions`` query options
    /// @return Reply fifo channel
    template<FifoChannelType C>
    ReplyFifoChannel<C> get_reply_fifo_channel(
        const KeyExpr& key_expr, const std::string& parameters, size_t bound, GetOptions&& options = GetOptions::create_default(), ZError* err = nullptr
    ) const {
        ::z_owned_reply_channel_t reply_channel;
        if constexpr (C == FifoChannelType::Blocking) {
            ::zc_reply_fifo_new(&reply_channel, bound);
        } else {
            ::zc_reply_non_blocking_fifo_new(&reply_channel, bound);
        }
        ReplyFifoChannel<C> recv(&reply_channel.recv);
        ::z_get_options_t opts;
        opts.target = options.target;
        opts.consolidation = static_cast<const z_query_consolidation_t&>(options.consolidation);
        opts.payload = detail::as_owned_c_ptr(options.payload);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.attachment = detail::as_owned_c_ptr(options.attachment);
        opts.timeout_ms = options.timeout_ms;

        ZError res = ::z_get(this->loan(), detail::loan(key_expr), parameters.c_str(), ::z_move(reply_channel.send), &opts);
        __ZENOH_ERROR_CHECK(
            res,
            err,
            "Failed to perform get operation"
        );
        if (res != Z_OK) std::move(recv).take();
        return recv;
    }
    /// @brief Options to be passed to ``delete_resource()`` operation
    struct DeleteOptions {
        /// @brief The priority of the delete message.
        Priority priority = Priority::Z_PRIORITY_DATA;
        /// @brief The congestion control to apply when routing delete message.
        CongestionControl congestion_control = CongestionControl::Z_CONGESTION_CONTROL_DROP;
        /// @brief Whether Zenoh will NOT wait to batch delete message with others to reduce the bandwith.
        bool is_express = false;

        /// @brief Returns default option settings
        static DeleteOptions create_default() { return {}; }
    };

    /// @brief Undeclare a resource. Equal to ``Publisher::delete_resource``
    /// @param key_expr ``KeyExprView`` the key expression to delete the resource
    /// @param options ``DeleteOptions`` delete options
    /// @return 0 in case of success, negative error code otherwise
    ZError delete_resource(const KeyExpr& key_expr, DeleteOptions&& options = DeleteOptions::create_default()) const {
        ::z_delete_options_t opts;
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;

        return ::z_delete(this->loan(), detail::loan(key_expr), &opts);
    }

    /// @brief Options passed to the ``put()`` operation
    struct PutOptions {
        /// @brief The priority of this message.
        Priority priority = Priority::Z_PRIORITY_DATA;
        /// @brief The congestion control to apply when routing this message.
        CongestionControl congestion_control = CongestionControl::Z_CONGESTION_CONTROL_DROP;
        /// @brief Whether Zenoh will NOT wait to batch this message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief  An optional encoding of the message payload and/or attachment.
        std::optional<Encoding> encoding = {};
        /// @brief An optional attachment to the message.
        std::optional<Bytes> attachment = {};

        /// @brief Returns default option settings
        static PutOptions create_default() { return {}; }
    };

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param key_expr The key expression to put the data
    /// @param payload The data to publish
    /// @param options Options to pass to put operation
    void put(const KeyExpr& key_expr, Bytes&& payload, PutOptions&& options = PutOptions::create_default(), ZError* err = nullptr) const {
        ::z_put_options_t opts;
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;
        opts.attachment = detail::as_owned_c_ptr(options.attachment);

        auto payload_ptr = detail::as_owned_c_ptr(payload);
        __ZENOH_ERROR_CHECK(
            ::z_put(this->loan(), detail::loan(key_expr), payload_ptr, &opts),
            err,
            "Failed to perform put operation"
        );
    }

    /// @brief Options to be passed when declaring a ``Queryable``
    struct QueryableOptions {
        /// @brief The completeness of the Queryable.
        bool complete = false;

        /// @brief Returns default option settings
        static QueryableOptions create_default() { return {}; }
    };

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests
    /// @param key_expr The key expression to match the ``Session::get`` requests
    /// @param on_query The callback to handle ``Query`` requests. Will be called once for each query
    /// @param on_drop The drop callback. Will be called once, when ``Queryable`` is destroyed or undeclared
    /// @param options Options passed to queryable declaration
    /// @return a ``Queryable`` object
    template<class C, class D>
    Queryable declare_queryable(
        const KeyExpr& key_expr, C&& on_query, D&& on_drop, QueryableOptions&& options = QueryableOptions::create_default(), ZError* err = nullptr
    ) const {
        static_assert(
            std::is_invocable_r<void, C, const Query&>::value,
            "on_query should be callable with the following signature: void on_query(const zenoh::Query& query)"
        );
        static_assert(
            std::is_invocable_r<void, D>::value,
            "on_drop should be callable with the following signature: void on_drop()"
        );
        ::z_owned_closure_query_t c_closure;
        using ClosureType = typename detail::closures::Closure<C, D, void, const Query&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_query), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_query_call, detail::closures::_zenoh_on_drop, closure);
        ::z_queryable_options_t opts;
        opts.complete = options.complete;

        Queryable q(nullptr);
        ZError res =  ::z_declare_queryable(
            detail::as_owned_c_ptr(q), this->loan(), detail::loan(key_expr), ::z_move(c_closure), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Queryable");
        return q;
    }

    /// @brief Options to be passed when declaring a ``Subscriber``
    struct SubscriberOptions {
        /// @brief The subscription reliability.
        Reliability reliability = Reliability::Z_RELIABILITY_BEST_EFFORT;

        /// @brief Returns default option settings
        static SubscriberOptions create_default() { return {}; }
    };

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put`` and ``Session::delete_resource`` requests
    /// @param key_expr The key expression to match the publishers
    /// @param on_sample The callback that will be called for each received sample
    /// @param on_drop The callback that will be called once subscriber is destroyed or undeclared
    /// @param options Options to pass to subscriber declaration
    /// @return a ``Subscriber`` object
    template<class C, class D>
    Subscriber declare_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop, SubscriberOptions&& options = SubscriberOptions::create_default(), ZError *err = nullptr
    ) const {
        static_assert(
            std::is_invocable_r<void, C, const Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(const zenoh::Sample& sample)"
        );
        static_assert(
            std::is_invocable_r<void, D>::value,
            "on_drop should be callable with the following signature: void on_drop()"
        );
        ::z_owned_closure_sample_t c_closure;
        using ClosureType = typename detail::closures::Closure<C, D, void, const Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_sample_call, detail::closures::_zenoh_on_drop, closure);
        ::z_subscriber_options_t opts;
        opts.reliability = options.reliability;
        Subscriber s(nullptr);
        ZError res =  ::z_declare_subscriber(
            detail::as_owned_c_ptr(s), this->loan(), detail::loan(key_expr), ::z_move(c_closure), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Subscriber");
        return s;
    }

    /// @brief Options to be passed when declaring a ``Publisher``
    struct PublisherOptions {
        /// @brief The congestion control to apply when routing messages from this publisher.
        CongestionControl congestion_control;
        /// @brief The priority of messages from this publisher.
        Priority priority;
        /// @brief If true, Zenoh will not wait to batch this message with others to reduce the bandwith
        bool is_express;
        /// @brief Returns default option settings

        static PublisherOptions create_default() { return {}; }
    };

    /// @brief Create a ``Publisher`` object to publish data to matching ``Subscriber`` and ``PullSubscriber`` objects
    /// @param key_expr The key expression to match the subscribers
    /// @param options Options passed to publisher declaration
    /// @return a ``Publisher`` object
    Publisher declare_publisher(
        const KeyExpr& key_expr, PublisherOptions&& options = PublisherOptions::create_default(), ZError* err = nullptr
    ) const {
        ::z_publisher_options_t opts;
        opts.congestion_control = options.congestion_control;
        opts.priority = options.priority;
        opts.is_express = options.is_express;

        Publisher p(nullptr);
        ZError res =  ::z_declare_publisher(
            detail::as_owned_c_ptr(p), this->loan(), detail::loan(key_expr), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Publisher");
        return p;
    }

    /// @brief Fetches the Zenoh IDs of all connected routers.
    std::vector<Id> get_routers_z_id(ZError* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const Id& z_id) {
            out.push_back(z_id);
        };
        typedef decltype(f) F;
        ::z_owned_closure_zid_t c_closure;
        using ClosureType = typename detail::closures::Closure<F, closures::None, void, const Id&>;
        auto closure = ClosureType::into_context(std::forward<F>(f), closures::none);
        ::z_closure(&c_closure, detail::closures::_zenoh_on_id_call,  detail::closures::_zenoh_on_drop, closure);
        __ZENOH_ERROR_CHECK(
            ::z_info_routers_zid(this->loan(), &c_closure),
            err,
            "Failed to fetch router Ids"
        );
        return out;
    }

    /// @brief Fetches the Zenoh IDs of all connected peers.
    std::vector<Id> get_peers_z_id(ZError* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const Id& z_id) {
            out.push_back(z_id);
        };
        typedef decltype(f) F;
        ::z_owned_closure_zid_t c_closure;
        auto closure = detail::closures::Closure<F, closures::None, void, const Id&>::into_context(
            std::forward<F>(f), closures::none
        );
        ::z_closure(&c_closure, detail::closures::_zenoh_on_id_call,  detail::closures::_zenoh_on_drop, closure);
        __ZENOH_ERROR_CHECK(
            ::z_info_peers_zid(this->loan(), &c_closure),
            err,
            "Failed to fetch peer Ids"
        );
        return out;
    }

#ifdef ZENOHCXX_ZENOHPICO

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task();

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task(ZError* error = nullptr);

    /// @brief Stop the read task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task();

    /// @brief Stop the read task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task(ZError* error = nullptr);

    /// @brief Start a separate task to handle the session lease.  This task will send KeepAlive messages when needed
    /// and will close the session when the lease is expired. When operating over a multicast transport, it also
    /// periodically sends the Join messages.
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_lease_task();

    /// @brief Start a separate task to handle the session lease. This task will send KeepAlive messages when needed
    /// and will close the session when the lease is expired. When operating over a multicast transport, it also
    /// periodically sends the Join messages.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_lease_task(ZError* error = nullptr);

    /// @brief Stop the lease task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task();

    /// @brief Stop the lease task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task(ZError* error = nullptr);

    /// @brief Triggers a single execution of reading procedure from the network and processes of any received the
    /// message
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool read();

    /// @brief Triggers a single execution of reading procedure from the network and processes of any received the
    /// message
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool read(ZError* error = nullptr);

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive();

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive(ZError* error = nullptr);

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join();

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join(ZError* error = nullptr);
#endif

    /// @brief Create a ``Session`` with the given ``Config``
    /// @param config ``Config`` to use
    /// @param start_background_tasks for zenoh-pico only. If true, start background threads which handles the network
    /// traffic. If false, the threads should be called manually with ``Session::start_read_task`` and
    /// ``Session::start_lease_task`` or methods ``Session::read``, ``Session::send_keep_alive`` and
    /// ``Session::send_join`` should be called in loop.
    /// @return a ``Session`` if the session was successfully created, an ``zenoh::ErrorMessage`` otherwise
    static Session open(Config&& config, ZError* err = nullptr) {
        return Session(std::move(config), err);
    }
};
}