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
#include "enums.hxx"
#include "keyexpr.hxx"
#include "bytes.hxx"
#include "value.hxx"

#include <string_view>
#include <optional>


namespace zenoh {
/// The query to be answered by a ``Queryable``
class Query : public Owned<::z_owned_query_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Get the key expression of the query
    /// @return ``KeyExpr`` value
    decltype(auto) get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_query_keyexpr(this->loan())); }

    /// @brief Get a query's <a href=https://github.com/eclipse-zenoh/roadmap/tree/main/rfcs/ALL/Selectors>parameters</a>
    ///
    /// @return Parameters string
    std::string_view get_parameters() const {
        ::z_view_string_t p;
        ::z_query_parameters(this->loan(), &p);
        return std::string_view(::z_string_data(::z_loan(p)), ::z_string_len(::z_loan(p))); 
    }

    /// @brief Get the value of the query (payload and encoding)
    /// @return ``Value`` value
    decltype(auto) get_value() const { return detail::as_owned_cpp_obj<Value>(::z_query_value(this->loan())); }

    /// @brief Checks if query contains an attachment
    /// @return ``True`` if query contains an attachment
    bool has_attachment() const { return ::z_query_attachment(this->loan()) != nullptr; }

    /// @brief Get the attachment of the query
    /// @return Attachment
    decltype(auto) get_attachment() const { return detail::as_owned_cpp_obj<Bytes>(::z_query_attachment(this->loan())); }

    /// @brief Options passed to the ``Query::reply`` operation
    struct ReplyOptions {
        /// @brief An optional encoding of this reply payload and/or attachment
        std::optional<Encoding> encoding = {};
        /// @brief An optional attachment to this reply.
        std::optional<Bytes> attachment = {};

        /// @brief Returns default option settings
        static ReplyOptions create_default() { return {}; }
    };

    /// @brief Send reply to a query
    void reply(const KeyExpr& key_expr, Bytes&& payload, ReplyOptions&& options = ReplyOptions::create_default(), ZError* err = nullptr) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_options_t opts;
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.attachment = detail::as_owned_c_ptr(options.attachment);

        __ZENOH_ERROR_CHECK(
            ::z_query_reply(this->loan(), detail::loan(key_expr), payload_ptr, &opts),
            err,
            "Failed to send reply"
        );
    }

    /// @brief Options passed to the ``Query::reply_err()`` operation
    struct ReplyErrOptions {
        /// @brief An optional encoding of the reply error payload
        std::optional<Encoding> encoding = {};

        /// @brief Returns default option settings
        static ReplyErrOptions create_default() { return {}; }
    };

    /// @brief Send error to a query
    void reply_err(Bytes&& payload, ReplyErrOptions&& options = ReplyErrOptions::create_default(), ZError* err = nullptr) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_err_options_t opts;
        opts.encoding = detail::as_owned_c_ptr(options.encoding);

        __ZENOH_ERROR_CHECK(
            ::z_query_reply_err(this->loan(), payload_ptr, &opts),
            err,
            "Failed to send error"
        );
    }
};

}