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

#include <algorithm>
#include <optional>
#include <variant>

#include "../base.hxx"
#include "../enums.hxx"
#include "../keyexpr.hxx"
#include "../matching.hxx"
#include "../query_consolidation.hxx"
#include "../session.hxx"
#include "advanced_publisher.hxx"
#include "advanced_subscriber.hxx"
#include "publication_cache.hxx"
#include "querying_subscriber.hxx"

namespace zenoh::ext {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
/// release.
/// @brief Zenoh Session interface extension.
/// @note Zenoh-c only.
class SessionExt {
    const zenoh::Session& _session;

   public:
    /// @name Constructors

    /// @brief Get extension interface for `zenoh::Session`. Its lifetime is bound that of the session.
    /// @param session
    SessionExt(const zenoh::Session& session) : _session(session){};

    /// @name Methods

    /// @warning This API is deprecated. Please use zenoh::ext::AdvancedPublisher.
    /// @brief Options passed to the ``SessionExt::declare_publication_cache``.
    struct PublicationCacheOptions {
        /// The suffix used for queryable.
        std::optional<KeyExpr> queryable_suffix = {};
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

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_publication_cache_options_t to_c_opts() {
            ::ze_publication_cache_options_t opts;
            ze_publication_cache_options_default(&opts);
            opts.queryable_suffix = zenoh::interop::as_loaned_c_ptr(this->queryable_suffix);
#if defined(Z_FEATURE_UNSTABLE_API)
            opts.queryable_origin = this->queryable_origin;
#endif
            opts.queryable_complete = this->queryable_complete;
            opts.history = this->history;
            opts.resources_limit = this->resources_limit;
            return opts;
        }
    };

    /// @warning This API is deprecated. Please use zenoh::ext::SessionExt::declare_advanced_publisher.
    /// @brief Construct and declare a publication cache.
    /// @param key_expr: The key expression to publish to.
    /// @param options: Additional options for the publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return declared ``PublicationCache`` instance.
    [[deprecated("Use declare_advanced_publisher instead.")]] [[nodiscard]] PublicationCache declare_publication_cache(
        const KeyExpr& key_expr, PublicationCacheOptions&& options = PublicationCacheOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        ::ze_publication_cache_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);

        PublicationCache p = zenoh::interop::detail::null<ext::PublicationCache>();
        zenoh::ZResult res = ::ze_declare_publication_cache(zenoh::interop::as_loaned_c_ptr(this->_session),
                                                            zenoh::interop::as_owned_c_ptr(p),
                                                            zenoh::interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Publication Cache");
        return p;
    }

    /// @warning This API is deprecated. Please use zenoh::ext::SessionExt::declare_advanced_publisher.
    /// @brief Declare a background publication cache. It will function in background until the corresponding session
    /// is closed or destoryed.
    /// @param key_expr the key expression to publish to.
    /// @param options additional options for the publication cache.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    [[deprecated]]
    void declare_background_publication_cache(
        const KeyExpr& key_expr, PublicationCacheOptions&& options = PublicationCacheOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        ::ze_publication_cache_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        zenoh::ZResult res = ::ze_declare_background_publication_cache(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Publication Cache");
    }

    /// @warning This API is deprecated. Please use zenoh::ext::AdvancedSubscriber.
    /// @brief Options passed to the ``SessionExt::declare_querying_subscriber``.
    struct QueryingSubscriberOptions {
        /// @name Fields

        /// The key expression to be used for queries.
        std::optional<KeyExpr> query_keyexpr = {};
        /// The restriction for the matching publications that will be received by this publication cache.
        zenoh::Locality allowed_origin = ::zc_locality_default();
#if defined(Z_FEATURE_UNSTABLE_API)
        /// The accepted replies for queries.
        zenoh::ReplyKeyExpr query_accept_replies = ::zc_reply_keyexpr_default();
#endif
        /// @brief The target to be used for queries.
        zenoh::QueryTarget query_target = QueryTarget::Z_QUERY_TARGET_BEST_MATCHING;
        /// @brief The consolidation mode to be used for queries.
        zenoh::QueryConsolidation query_consolidation =
            QueryConsolidation(ConsolidationMode::Z_CONSOLIDATION_MODE_NONE);
        /// @brief The timeout to be used for queries.
        uint64_t query_timeout_ms = 0;

        /// @name Methods

        /// @brief Create default option settings.
        static QueryingSubscriberOptions create_default() { return {}; }

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_querying_subscriber_options_t to_c_opts() {
            ::ze_querying_subscriber_options_t opts;
            ze_querying_subscriber_options_default(&opts);
            opts.query_selector = zenoh::interop::as_loaned_c_ptr(this->query_keyexpr);
            opts.allowed_origin = this->allowed_origin;
#if defined(Z_FEATURE_UNSTABLE_API)
            opts.query_accept_replies = this->query_accept_replies;
#endif
            opts.query_target = this->query_target;
            opts.query_consolidation = *zenoh::interop::as_copyable_c_ptr(this->query_consolidation);
            opts.query_timeout_ms = this->query_timeout_ms;
            return opts;
        };
    };

    /// @warning This API is deprecated. Please use zenoh::ext::SessionExt::declare_advanced_subscriber.
    /// @brief Construct and declare a querying subscriber.
    /// @param key_expr the key expression to subscribe to.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options additional options for querying subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return declared ``QueryingSubscriber`` instance.
    template <class C, class D>
    [[nodiscard]] [[deprecated("Use declare_advanced_subscriber instead.")]] QueryingSubscriber<void>
    declare_querying_subscriber(const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
                                QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
                                zenoh::ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, zenoh::Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::ze_querying_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, zenoh::Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        QueryingSubscriber<void> qs = zenoh::interop::detail::null<QueryingSubscriber<void>>();
        zenoh::ZResult res = ::ze_declare_querying_subscriber(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_owned_c_ptr(qs),
            zenoh::interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Querying Subscriber");
        return qs;
    }

    /// @warning This API is deprecated. Please use zenoh::ext::SessionExt::declare_background_advanced_subscriber.
    /// @brief Declare a background querying subscriber for a given key expression. Subscriber callback will be called
    /// to process the messages, until the corresponding session is closed or dropped.
    /// @param key_expr the key expression to subscribe to.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options additional options for querying subscriber.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    [[deprecated("Use declare_background_advanced_subscriber instead.")]]
    void declare_background_querying_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, zenoh::Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const zenoh::Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        ::ze_querying_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        zenoh::ZResult res = ::ze_declare_background_querying_subscriber(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_loaned_c_ptr(key_expr),
            ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Querying Subscriber");
    }

    /// @warning This API is deprecated. Please use zenoh::ext::SessionExt::declare_advanced_subscriber.
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
    [[deprecated("Use declare_advanced_subscriber instead.")]] [[nodiscard]] QueryingSubscriber<
        typename Channel::template HandlerType<zenoh::Sample>>
    declare_querying_subscriber(const KeyExpr& key_expr, Channel channel,
                                QueryingSubscriberOptions&& options = QueryingSubscriberOptions::create_default(),
                                zenoh::ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Sample>();
        ::ze_querying_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        QueryingSubscriber<void> qs = zenoh::interop::detail::null<QueryingSubscriber<void>>();
        zenoh::ZResult res = ::ze_declare_querying_subscriber(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_owned_c_ptr(qs),
            zenoh::interop::as_loaned_c_ptr(key_expr), ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Querying Subscriber");
        if (res != Z_OK) ::z_drop(zenoh::interop::as_moved_c_ptr(cb_handler_pair.second));
        return QueryingSubscriber<typename Channel::template HandlerType<zenoh::Sample>>(
            std::move(qs), std::move(cb_handler_pair.second));
    }

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// Options passed to the `SessionExt::declare_advanced_publisher()` function.
    struct AdvancedPublisherOptions {
        /// @brief  Setting for advanced publisher's cache. The cache allows advanced subscribers to recover history
        /// and/or lost samples.
        struct CacheOptions {
            /// @name Fields

            /// Number of samples to keep for each resource.
            size_t max_samples = 1;
            /// The congestion control to apply to replies.
            zenoh::CongestionControl congestion_control = ::z_internal_congestion_control_default_response();
            /// The priority of replies.
            zenoh::Priority priority = ::z_priority_default();
            /// If set to ``true``, this cache replies will not be batched. This usually has a positive impact on
            /// latency but negative impact on throughput.
            bool is_express = false;

            /// @name Methods

            /// @brief Create default option settings.
            static CacheOptions create_default() { return {}; }
        };

        /// @brief Settings allowing matching Subscribers to detect lost samples and optionally ask for retransimission.
        struct SampleMissDetectionOptions {
            /// @brief Disable last sample miss detection through heartbeat.
            struct HeartbeatNone {};

            /// @brief Allow last sample miss detection through periodic heartbeat.
            /// Periodically send the last published Sample's sequence number to allow last sample
            /// miss detection.
            struct HeartbeatPeriodic {
                /// @name Fields
                /// @brief The period of publisher periodic heartbeats in ms.
                uint64_t period_ms;
            };

            /// @brief Allow last sample miss detection through sporadic heartbeat.
            /// Each period, the last published Sample's sequence number is sent with `z_congestion_control_t::BLOCK`
            /// but only if it changed since last period.
            struct HeartbeatSporadic {
                /// @name Fields
                /// @brief The period of publisher sporadic heartbeats in ms.
                uint64_t period_ms;
            };

            /// @name Fields

            /// Configure last sample miss detection through sporadic or periodic heartbeat.
            std::variant<HeartbeatNone, HeartbeatPeriodic, HeartbeatSporadic> heartbeat = HeartbeatNone{};

            /// @name Methods

            /// @brief Create default option settings.
            static SampleMissDetectionOptions create_default() { return {}; }
        };

        /// @name Fields

        /// Base publisher options.
        zenoh::Session::PublisherOptions publisher_options = {};
        /// Optional settings for publisher cache.
        std::optional<CacheOptions> cache;
        /// Allow matching Subscribers to detect lost samples and optionally ask for retransimission.
        ///
        /// Retransmission can only be done if cache is enabled.
        std::optional<SampleMissDetectionOptions> sample_miss_detection = {};
        /// Allow this publisher to be detected through liveliness.
        bool publisher_detection = false;
        /// An optional key expression to be added to the liveliness token key expression.
        /// It can be used to convey meta data.
        std::optional<zenoh::KeyExpr> publisher_detection_metadata = {};

        /// @name Methods

        /// @brief Create default option settings.
        static AdvancedPublisherOptions create_default() { return {}; }

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_advanced_publisher_options_t to_c_opts() {
            ::ze_advanced_publisher_options_t opts;
            ::ze_advanced_publisher_options_default(&opts);
            opts.publisher_options = zenoh::interop::detail::Converter::to_c_opts(this->publisher_options);
            if (this->cache.has_value()) {
                opts.cache.is_enabled = true;
                opts.cache.max_samples = this->cache->max_samples;
                opts.cache.congestion_control = this->cache->congestion_control;
                opts.cache.priority = this->cache->priority;
                opts.cache.is_express = this->cache->is_express;
            }
            opts.publisher_detection = this->publisher_detection;
            if (this->sample_miss_detection.has_value()) {
                opts.sample_miss_detection.is_enabled = true;
                if (std::holds_alternative<SampleMissDetectionOptions::HeartbeatPeriodic>(
                        this->sample_miss_detection->heartbeat)) {
                    opts.sample_miss_detection.heartbeat_mode =
                        ::ze_advanced_publisher_heartbeat_mode_t::ZE_ADVANCED_PUBLISHER_HEARTBEAT_MODE_PERIODIC;
                    opts.sample_miss_detection.heartbeat_period_ms =
                        std::get<SampleMissDetectionOptions::HeartbeatPeriodic>(this->sample_miss_detection->heartbeat)
                            .period_ms;
                } else if (std::holds_alternative<SampleMissDetectionOptions::HeartbeatSporadic>(
                               this->sample_miss_detection->heartbeat)) {
                    opts.sample_miss_detection.heartbeat_mode =
                        ::ze_advanced_publisher_heartbeat_mode_t::ZE_ADVANCED_PUBLISHER_HEARTBEAT_MODE_SPORADIC;
                    opts.sample_miss_detection.heartbeat_period_ms =
                        std::get<SampleMissDetectionOptions::HeartbeatSporadic>(this->sample_miss_detection->heartbeat)
                            .period_ms;
                } else {
                    opts.sample_miss_detection.heartbeat_mode =
                        ::ze_advanced_publisher_heartbeat_mode_t::ZE_ADVANCED_PUBLISHER_HEARTBEAT_MODE_NONE;
                }
            }
            opts.publisher_detection_metadata = zenoh::interop::as_loaned_c_ptr(this->publisher_detection_metadata);
            return opts;
        }
    };

    /// @brief Create an ``AdvancedPublisher`` object to publish data to matching ``zenoh::Subscriber``or
    /// ``AdvancedSubscriber``  objects.
    /// @param key_expr the key expression to match the subscribers.
    /// @param options options passed to advanced publisher declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return an ``AdvancedPublisher`` object.
    AdvancedPublisher declare_advanced_publisher(
        const KeyExpr& key_expr, AdvancedPublisherOptions&& options = AdvancedPublisherOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        ::ze_advanced_publisher_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        AdvancedPublisher p = zenoh::interop::detail::null<AdvancedPublisher>();
        zenoh::ZResult res = ::ze_declare_advanced_publisher(zenoh::interop::as_loaned_c_ptr(this->_session),
                                                             zenoh::interop::as_owned_c_ptr(p),
                                                             zenoh::interop::as_loaned_c_ptr(key_expr), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Advanced Publisher");
        return p;
    }

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Options passed to the ``SessionExt::declare_advanced_subscriber``.
    struct AdvancedSubscriberOptions {
        /// @name Fields

        /// @brief Settings for retrievieng historical data. History can only be retransmitted by Publishers that enable
        /// caching.
        struct HistoryOptions {
            /// @name Fields

            /// Enable detection of late joiner publishers and query for their historical data.
            /// Late joiner detection can only be achieved for Publishers that enable publisher_detection.
            bool detect_late_publishers = false;
            /// Number of samples to query for each resource. ``0`` corresponds to no limit on number of samples.
            size_t max_samples = 0;
            /// Maximum age of samples to query. ``0`` corresponds to no limit on samples' age.
            uint64_t max_age_ms = 0;

            /// @name Methods

            /// @brief Create default option settings.
            static HistoryOptions create_default() { return {}; };
        };

        /// @brief Settings for recovering lost messages for Advanced Subscriber.
        struct RecoveryOptions {
            /// @name Fields

            /// @brief  Option tag for Heartbeat-based last sample detection.
            struct Heartbeat {};

            /// @brief Settings for periodic queries-based last sample detection.
            struct PeriodicQueriesOptions {
                /// @name Fields

                /// @brief Period for queries for not yet received Samples.
                ///
                /// These queries allow to retrieve the last Sample(s) if the last Sample(s) is/are lost.
                /// So it is useful for sporadic publications but useless for periodic publications
                /// with a period smaller or equal to this period.
                uint64_t period_ms = 1000;

                /// @name Methods

                /// @brief Create default option settings.
                static PeriodicQueriesOptions create_default() { return {}; };
            };

            /// @brief Setting for detecting last sample(s) miss.
            /// Note that it does not affect intermediate sample miss detection/retrieval (which is performed
            /// automatically as long as recovery is enabled). If this option is disabled, subscriber will be unable to
            /// detect/request retransmission of missed sample until it receives a more recent one from the same
            /// publisher.
            std::optional<std::variant<Heartbeat, PeriodicQueriesOptions>> last_sample_miss_detection = {};

            /// @name Methods

            /// @brief Create default option settings.
            static RecoveryOptions create_default() { return {}; };
        };

        /// Base subscriber options.
        zenoh::Session::SubscriberOptions subscriber_options;
        /// Optional settings for querying historical data. History can only be retransmitted by Publishers that enable
        /// caching.
        std::optional<HistoryOptions> history = {};
        /// Optional settings for retransmission of detected lost Samples. Retransmission of lost samples can only be
        /// done by Publishers that enable caching and sample_miss_detection.
        std::optional<RecoveryOptions> recovery = {};
        /// Timeout to be used for history and recovery queries.
        /// Default value will be used if set to ``0``.
        uint64_t query_timeout_ms = 0;
        /// Allow this subscriber to be detected through liveliness.
        bool subscriber_detection = false;
        /// An optional key expression to be added to the liveliness token key expression.
        /// It can be used to convey meta data.
        std::optional<zenoh::KeyExpr> subscriber_detection_metadata = {};

        /// @name Methods

        /// @brief Create default option settings.
        static AdvancedSubscriberOptions create_default() { return {}; }

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_advanced_subscriber_options_t to_c_opts() {
            ::ze_advanced_subscriber_options_t opts;
            ::ze_advanced_subscriber_options_default(&opts);
            opts.subscriber_options = zenoh::interop::detail::Converter::to_c_opts(this->subscriber_options);
            if (this->history.has_value()) {
                opts.history.is_enabled = true;
                opts.history.detect_late_publishers = this->history->detect_late_publishers;
                opts.history.max_samples = this->history->max_samples;
                opts.history.max_age_ms = this->history->max_age_ms;
            }
            if (this->recovery.has_value()) {
                opts.recovery.is_enabled = true;
                if (this->recovery->last_sample_miss_detection.has_value()) {
                    opts.recovery.last_sample_miss_detection.is_enabled = true;
                    if (std::holds_alternative<RecoveryOptions::Heartbeat>(
                            this->recovery->last_sample_miss_detection.value())) {
                        opts.recovery.last_sample_miss_detection.periodic_queries_period_ms = 0;
                    } else {
                        // treat 0 as very small delay
                        opts.recovery.last_sample_miss_detection.periodic_queries_period_ms =
                            std::max<uint64_t>(1, std::get<RecoveryOptions::PeriodicQueriesOptions>(
                                                      this->recovery->last_sample_miss_detection.value())
                                                      .period_ms);
                    }
                }
            }
            opts.query_timeout_ms = this->query_timeout_ms;
            opts.subscriber_detection = this->subscriber_detection;
            opts.subscriber_detection_metadata = zenoh::interop::as_loaned_c_ptr(this->subscriber_detection_metadata);
            return opts;
        }
    };

    /// @brief Create an ``AdvancedSubscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put`` and ``Session::delete_resource`` requests.
    /// @param key_expr the key expression to match the publishers.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class C, class D>
    [[nodiscard]] AdvancedSubscriber<void> declare_advanced_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        AdvancedSubscriberOptions&& options = AdvancedSubscriberOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, const Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        ::ze_advanced_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        AdvancedSubscriber<void> s = zenoh::interop::detail::null<AdvancedSubscriber<void>>();
        zenoh::ZResult res = ::ze_declare_advanced_subscriber(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_owned_c_ptr(s),
            zenoh::interop::as_loaned_c_ptr(key_expr), ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Advanced Subscriber");
        return s;
    }

    /// @brief Declare a background advanced subscriber. It will receive data from matching ``Publisher`` objects or
    /// from
    /// ``Session::put`` and ``Session::delete_resource`` requests, until the corresponding session is closed or
    /// destroyed.
    /// @param key_expr the key expression to match the publishers.
    /// @param on_sample the callback that will be called for each received sample.
    /// @param on_drop the callback that will be called once subscriber is destroyed or undeclared.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void declare_background_advanced_subscriber(
        const KeyExpr& key_expr, C&& on_sample, D&& on_drop,
        AdvancedSubscriberOptions&& options = AdvancedSubscriberOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        static_assert(
            std::is_invocable_r<void, C, const zenoh::Sample&>::value,
            "on_sample should be callable with the following signature: void on_sample(zenoh::Sample& sample)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_sample_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const zenoh::Sample&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_sample), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_sample_call, zenoh::detail::closures::_zenoh_on_drop,
                    closure);
        ::ze_advanced_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        ZResult res = ::ze_declare_background_advanced_subscriber(zenoh::interop::as_loaned_c_ptr(this->_session),
                                                                  zenoh::interop::as_loaned_c_ptr(key_expr),
                                                                  ::z_move(c_closure), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Background Advanced Subscriber");
    }

    /// @brief Create an ``AdvancedSubscriber`` object to receive data from matching ``Publisher`` objects or from.
    /// ``Session::put`` and ``Session::delete_resource`` requests.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param key_expr the key expression to match the publishers.
    /// @param channel an instance of channel.
    /// @param options options to pass to subscriber declaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``Subscriber`` object.
    template <class Channel>
    [[nodiscard]] AdvancedSubscriber<typename Channel::template HandlerType<Sample>> declare_advanced_subscriber(
        const KeyExpr& key_expr, Channel channel,
        AdvancedSubscriberOptions&& options = AdvancedSubscriberOptions::create_default(),
        zenoh::ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Sample>();
        ::ze_advanced_subscriber_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        AdvancedSubscriber<void> s = zenoh::interop::detail::null<AdvancedSubscriber<void>>();
        zenoh::ZResult res = ::ze_declare_advanced_subscriber(
            zenoh::interop::as_loaned_c_ptr(this->_session), zenoh::interop::as_owned_c_ptr(s),
            zenoh::interop::as_loaned_c_ptr(key_expr), ::z_move(cb_handler_pair.first), &opts);
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Advanced Subscriber");
        if (res != Z_OK) ::z_drop(zenoh::interop::as_moved_c_ptr(cb_handler_pair.second));
        return AdvancedSubscriber<typename Channel::template HandlerType<Sample>>(std::move(s),
                                                                                  std::move(cb_handler_pair.second));
    }
};
}  // namespace zenoh::ext

#endif
