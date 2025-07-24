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

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1
#include <functional>
#include <optional>
#include <string_view>

#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif
#include "timestamp.hxx"

namespace zenoh {
/// The query to be answered by a ``Queryable``.
class Query : public Owned<::z_owned_query_t> {
    Query(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Get the key expression of the query.
    /// @return ``KeyExpr`` of the query.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_query_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Get query parameters. See <a
    /// href=https://github.com/eclipse-zenoh/roadmap/tree/main/rfcs/ALL/Selectors>Selector</a> for more information.
    ///
    /// @return parameters string.
    std::string_view get_parameters() const {
        ::z_view_string_t p;
        ::z_query_parameters(interop::as_loaned_c_ptr(*this), &p);
        return std::string_view(::z_string_data(::z_loan(p)), ::z_string_len(::z_loan(p)));
    }

    /// @brief Get the payload of the query.
    /// @return payload of the query.
    std::optional<std::reference_wrapper<const Bytes>> get_payload() const {
        auto payload = ::z_query_payload(interop::as_loaned_c_ptr(*this));
        if (payload == nullptr) return {};
        return std::cref(interop::as_owned_cpp_ref<Bytes>(payload));
    }
#if defined(ZENOHCXX_ZENOHC)
    /// @brief Get the payload of the query.
    /// @return payload of the query.
    std::optional<std::reference_wrapper<Bytes>> get_payload() {
        auto payload = ::z_query_payload_mut(interop::as_loaned_c_ptr(*this));
        if (payload == nullptr) return {};
        return std::ref(interop::as_owned_cpp_ref<Bytes>(payload));
    }
#endif

    /// @brief Get the encoding of the query.
    /// @return encoding of the query.
    std::optional<std::reference_wrapper<const Encoding>> get_encoding() const {
        auto encoding = ::z_query_encoding(interop::as_loaned_c_ptr(*this));
        if (encoding == nullptr) return {};
        return std::cref(interop::as_owned_cpp_ref<Encoding>(::z_query_encoding(interop::as_loaned_c_ptr(*this))));
    }

    /// @brief Get the attachment of the query.
    /// @return attachment of the query.
    std::optional<std::reference_wrapper<const Bytes>> get_attachment() const {
        auto attachment = ::z_query_attachment(interop::as_loaned_c_ptr(*this));
        if (attachment == nullptr) {
            return {};
        }
        return std::cref(interop::as_owned_cpp_ref<Bytes>(attachment));
    }

#if defined(ZENOHCXX_ZENOHC)
    /// @brief Get the mutable attachment of the query.
    /// @return attachment of the query.
    std::optional<std::reference_wrapper<Bytes>> get_attachment() {
        auto attachment = ::z_query_attachment_mut(interop::as_loaned_c_ptr(*this));
        if (attachment == nullptr) {
            return {};
        }
        return std::ref(interop::as_owned_cpp_ref<Bytes>(attachment));
    }
#endif

    /// @brief Options passed to the ``Query::reply`` operation.
    struct ReplyOptions {
        /// @name Fields

        /// @brief An optional encoding of the reply message payload and/or attachment.
        std::optional<Encoding> encoding = {};
        /// @brief The priority of this reply message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing this reply message.
        CongestionControl congestion_control = ::z_internal_congestion_control_default_response();
        /// @brief Whether Zenoh will NOT wait to batch this reply message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief The timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info of this reply message.
        /// @note Zenoh-c only.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief An optional attachment to this reply message.
        std::optional<Bytes> attachment = {};

        /// @name Methods

        /// @brief Create default option settings.
        static ReplyOptions create_default() { return {}; }
    };

    /// @brief Send reply to a query.
    /// @param options options to pass to reply operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply(const KeyExpr& key_expr, Bytes&& payload, ReplyOptions&& options = ReplyOptions::create_default(),
               ZResult* err = nullptr) const {
        auto payload_ptr = interop::as_moved_c_ptr(payload);
        ::z_query_reply_options_t opts;
        z_query_reply_options_default(&opts);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);
        opts.priority = options.priority;
        opts.congestion_control = options.congestion_control;
        opts.is_express = options.is_express;
        opts.timestamp = interop::as_copyable_c_ptr(options.timestamp);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);

        __ZENOH_RESULT_CHECK(
            ::z_query_reply(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), payload_ptr, &opts),
            err, "Failed to send reply");
    }

    /// @brief Options passed to the ``Query::reply_err`` operation.
    struct ReplyErrOptions {
        /// @name Fields.

        /// @brief An optional encoding of the reply error payload
        std::optional<Encoding> encoding = {};

        /// @name Methods

        /// @brief Create default option settings.
        static ReplyErrOptions create_default() { return {}; }
    };

    /// @brief Send error reply to a query.
    /// @param options options to pass to reply error operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply_err(Bytes&& payload, ReplyErrOptions&& options = ReplyErrOptions::create_default(),
                   ZResult* err = nullptr) const {
        auto payload_ptr = interop::as_moved_c_ptr(payload);
        ::z_query_reply_err_options_t opts;
        z_query_reply_err_options_default(&opts);
        opts.encoding = interop::as_moved_c_ptr(options.encoding);

        __ZENOH_RESULT_CHECK(::z_query_reply_err(interop::as_loaned_c_ptr(*this), payload_ptr, &opts), err,
                             "Failed to send reply error");
    }

    /// @brief Options passed to the ``Query::reply_del`` operation.
    struct ReplyDelOptions {
        /// @name Fields.

        /// @brief The priority of this reply message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing this reply message.
        CongestionControl congestion_control = ::z_internal_congestion_control_default_response();
        /// @brief Whether Zenoh will NOT wait to batch this reply message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief the timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info of this reply message.
        /// @note Zenoh-c only.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief An optional attachment to this reply message.
        std::optional<Bytes> attachment = {};

        /// @name Methods.

        /// @brief Create default option settings.
        static ReplyDelOptions create_default() { return {}; }
    };

    /// @brief Send a delete reply to a query.
    /// @param key_expr: the key of this delete reply.
    /// @param options: the options to pass to reply del operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply_del(const KeyExpr& key_expr, ReplyDelOptions&& options = ReplyDelOptions::create_default(),
                   ZResult* err = nullptr) const {
        ::z_query_reply_del_options_t opts;
        z_query_reply_del_options_default(&opts);
        opts.priority = options.priority;
        opts.congestion_control = options.congestion_control;
        opts.is_express = options.is_express;
        opts.timestamp = interop::as_copyable_c_ptr(options.timestamp);
#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
        opts.source_info = interop::as_moved_c_ptr(options.source_info);
#endif
        opts.attachment = interop::as_moved_c_ptr(options.attachment);

        __ZENOH_RESULT_CHECK(
            ::z_query_reply_del(interop::as_loaned_c_ptr(*this), interop::as_loaned_c_ptr(key_expr), &opts), err,
            "Failed to send reply del");
    }

    /// @brief Construct a shallow copy of this Query.
    /// This can be used, to send query replies outside of ``Queryable`` callback.
    ///
    /// The query responses will be sent only when the last clone is destroyed.
    Query clone() const {
        Query q(zenoh::detail::null_object);
        ::z_query_clone(&q._0, interop::as_loaned_c_ptr(*this));
        return q;
    };
};

}  // namespace zenoh
#endif
