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

#include <optional>

#include "../base.hxx"
#include "../enums.hxx"
#include "../keyexpr.hxx"
#include "../query_consolidation.hxx"
#include "../session.hxx"

namespace zenoh::ext {
template <class Handler>
class QueryingSubscriber;
class PublicationCache;

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
/// release.
/// @brief Zenoh Session interface extension.
/// @note Zenoh-c only.
class SessionExt {
    const ::zenoh::Session& _session;

   public:
    /// @name Constructors

    /// @brief Get extension interface for `::zenoh::Session`. Its lifetime is bound that of the session.
    /// @param session
    SessionExt(const zenoh::Session& session) : _session(session){};

    /// @name Methods

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Options passed to the ``SessionExt::declare_publication_cache``.
    struct PublicationCacheOptions {
        /// The prefix used for queryable.
        std::optional<KeyExpr> queryable_prefix = {};
#if defined(Z_FEATURE_UNSTABLE_API)
        /// The restriction for the matching queries that will be receive by this publication cache.
        Locality queryable_origin = ::zc_locality_default();
#endif
        /// The `complete` option for the queryable.
        bool queryable_complete = false;
        /// The the history size (i.e. maximum number of messages to store).
        size_t history = 1;
        /// The limit number of cached resources. ``0`` value corresponds to no limit.
        size_t resources_limit = 0;

        /// @name Methods

        /// @brief Create default option settings.
        static PublicationCacheOptions create_default() { return {}; }
    };

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Construct and declare a publication cache.
    /// @param key_expr: The key expression to publish to.
    /// @param options: Additional options for the publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return declared ``PublicationCache`` instance.
    [[nodiscard]] PublicationCache declare_publication_cache(
        const KeyExpr& key_expr, PublicationCacheOptions&& options = PublicationCacheOptions::create_default(),
        ZResult* err = nullptr) const;

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Declare a background publication cache. It will function in background until the corresponding session
    /// is closed or destoryed.
    /// @param key_expr the key expression to publish to.
    /// @param options additional options for the publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void declare_background_publication_cache(
        const KeyExpr& key_expr, PublicationCacheOptions&& options = PublicationCacheOptions::create_default(),
        ZResult* err = nullptr) const;

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Options passed to the ``Session::declare_querying_subscriber``.
    struct QueryingSubscriberOptions {
        /// @name Fields

        /// The key expression to be used for queries.
        std::optional<KeyExpr> query_keyexpr = {};
#if defined(Z_FEATURE_UNSTABLE_API)
        /// The restriction for the matching publications that will be received by this publication cache.
        Locality allowed_origin = ::zc_locality_default();
        /// The accepted replies for queries.
        ReplyKeyExpr query_accept_replies = ::zc_reply_keyexpr_default();
#endif
        /// @brief The target to be used for queries.
        QueryTarget query_target = QueryTarget::Z_QUERY_TARGET_BEST_MATCHING;
        /// @brief The consolidation mode to be used for queries.
        QueryConsolidation query_consolidation = QueryConsolidation(ConsolidationMode::Z_CONSOLIDATION_MODE_NONE);
        /// @brief The timeout to be used for queries.
        uint64_t query_timeout_ms = 0;

        /// @name Methods

        /// @brief Create default option settings.
        static QueryingSubscriberOptions create_default() { return {}; }
    };

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Construct and declare a querying subscriber.
    /// @param key_expr the key expression to subscribe to.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options additional options for querying subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return declared ``QueryingSubscriber`` instance.
    template <class C, class D>
    [[nodiscard]] QueryingSubscriber<void> declare_querying_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
        ZResult* err = nullptr) const;

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Declare a background querying subscriber for a given key expression. Subscriber callback will be called
    /// to process the messages, until the corresponding session is closed or dropped.
    /// @param key_expr the key expression to subscribe to.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options additional options for querying subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void declare_background_querying_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
        ZResult* err = nullptr) const;

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Construct and declare a querying subscriber.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param key_expr the key expression to subscriber to.
    /// @param channel an instance of channel.
    /// @param options options to pass to querying subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``QueryingSubscriber`` object.
    template <class Channel>
    [[nodiscard]] QueryingSubscriber<typename Channel::template HandlerType<Sample>> declare_querying_subscriber(
        const KeyExpr& key_expr, Channel channel,
        QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
        ZResult* err = nullptr) const;
};

}  // namespace zenoh::ext

#endif