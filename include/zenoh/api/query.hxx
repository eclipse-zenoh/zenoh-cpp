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
#include <string_view>

#include "../detail/interop.hxx"
#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "keyexpr.hxx"

namespace zenoh {
/// The query to be answered by a ``Queryable``.
class Query : public Owned<::z_owned_query_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Get the key expression of the query.
    /// @return ``KeyExpr`` of the query.
    const KeyExpr& get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_query_keyexpr(this->loan())); }

    /// @brief Get query parameters. See <a
    /// href=https://github.com/eclipse-zenoh/roadmap/tree/main/rfcs/ALL/Selectors>Selector</a> for more information.
    ///
    /// @return parameters string.
    std::string_view get_parameters() const {
        ::z_view_string_t p;
        ::z_query_parameters(this->loan(), &p);
        return std::string_view(::z_string_data(::z_loan(p)), ::z_string_len(::z_loan(p)));
    }

    /// @brief Get the payload of the query.
    /// @return payload of the query.
    const Bytes& get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_query_payload(this->loan())); }

    /// @brief Get the encoding of the query.
    /// @return encoding of the query.
    const Encoding& get_encoding() const {
        return detail::as_owned_cpp_obj<Encoding>(::z_query_encoding(this->loan()));
    }

    /// @brief Checks if query contains an attachment. Will throw a ZException if ``Reply::has_attachment`` returns
    /// ``false``.
    /// @return ``true`` if query contains an attachment.
    bool has_attachment() const { return ::z_query_attachment(this->loan()) != nullptr; }

    /// @brief Get the attachment of the query.
    ///
    /// Will throw a ZException if ``Query::has_attachment`` returns ``false``.
    /// @return attachment of the query.
    const Bytes& get_attachment() const {
        auto attachment = ::z_query_attachment(this->loan());
        if (attachment == nullptr) {
            throw ZException("Query does not contain an attachment", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Bytes>(attachment);
    }

    /// @brief Options passed to the ``Query::reply`` operation.
    struct ReplyOptions {
        /// @name Fields

        /// @brief  An optional encoding of the reply message payload and/or attachment.
        std::optional<Encoding> encoding = {};
        /// @brief The priority of this reply message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing this reply message.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief Whether Zenoh will NOT wait to batch this reply message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief The timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        /// @brief The source info of this reply message.
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
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply(const KeyExpr& key_expr, Bytes&& payload, ReplyOptions&& options = ReplyOptions::create_default(),
               ZResult* err = nullptr) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_options_t opts;
        z_query_reply_options_default(&opts);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.priority = options.priority;
        opts.congestion_control = options.congestion_control;
        opts.is_express = options.is_express;
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        opts.source_info = detail::as_owned_c_ptr(options.source_info);
#endif
        opts.attachment = detail::as_owned_c_ptr(options.attachment);

        __ZENOH_RESULT_CHECK(::z_query_reply(this->loan(), detail::loan(key_expr), payload_ptr, &opts), err,
                             "Failed to send reply");
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
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply_err(Bytes&& payload, ReplyErrOptions&& options = ReplyErrOptions::create_default(),
                   ZResult* err = nullptr) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_err_options_t opts;
        z_query_reply_err_options_default(&opts);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);

        __ZENOH_RESULT_CHECK(::z_query_reply_err(this->loan(), payload_ptr, &opts), err, "Failed to send reply error");
    }

    /// @brief Options passed to the ``Query::reply_del`` operation.
    struct ReplyDelOptions {
        /// @name Fields.

        /// @brief The priority of this reply message.
        Priority priority = Z_PRIORITY_DEFAULT;
        /// @brief The congestion control to apply when routing this reply message.
        CongestionControl congestion_control = Z_CONGESTION_CONTROL_DEFAULT;
        /// @brief Whether Zenoh will NOT wait to batch this reply message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief the timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        /// @brief The source info of this reply message.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief An optional attachment to this reply message.
        std::optional<Bytes> attachment = {};

        /// @name Methods.

        /// @brief Create default option settings.
        static ReplyDelOptions create_default() { return {}; }
    };

    /// @brief Send a delete reply to a query.
    /// @param key_expr: The key of this delete reply.
    /// @param options: The options to pass to reply del operation.
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void reply_del(const KeyExpr& key_expr, ReplyDelOptions&& options = ReplyDelOptions::create_default(),
                   ZResult* err = nullptr) const {
        ::z_query_reply_del_options_t opts;
        z_query_reply_del_options_default(&opts);
        opts.priority = options.priority;
        opts.congestion_control = options.congestion_control;
        opts.is_express = options.is_express;
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        opts.source_info = detail::as_owned_c_ptr(options.source_info);
#endif
        opts.attachment = detail::as_owned_c_ptr(options.attachment);

        __ZENOH_RESULT_CHECK(::z_query_reply_del(this->loan(), detail::loan(key_expr), &opts), err,
                             "Failed to send reply del");
    }
};

}  // namespace zenoh