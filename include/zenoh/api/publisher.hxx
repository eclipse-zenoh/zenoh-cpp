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

#include "../detail/interop.hxx"
#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "keyexpr.hxx"
#include "timestamp.hxx"
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
#include "source_info.hxx"
#endif
#include <optional>

namespace zenoh {
/// A Zenoh publisher. Constructed by ``Session::declare_publisher`` method.
class Publisher : public Owned<::z_owned_publisher_t> {
   public:
    using Owned::Owned;

    /// @brief Options to be passed to ``Publisher::put`` operation.
    struct PutOptions {
        /// @name Fields

        /// @brief The encoding of the data to publish.
        std::optional<Encoding> encoding = {};
        /// @brief the timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        /// @brief The source info of this message.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief The attachment to attach to the publication.
        std::optional<Bytes> attachment = {};

        /// @name Methods

        /// @brief Create default option settings.
        static PutOptions create_default() { return {}; }
    };

    /// @brief Options to be passed to ``Publisher::delete_resource`` operation
    struct DeleteOptions {
        /// @brief the timestamp of this message
        std::optional<Timestamp> timestamp = {};

        /// @brief Create default option settings.
        static DeleteOptions create_default() { return {}; }
    };

    /// @name Methods

    /// @brief Publish a message on publisher key expression.
    /// @param payload Data to publish.
    /// @param options Optional values passed to put operation.
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void put(Bytes&& payload, PutOptions&& options = PutOptions::create_default(), ZResult* err = nullptr) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_publisher_put_options_t opts;
        z_publisher_put_options_default(&opts);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
        opts.source_info = detail::as_owned_c_ptr(options.source_info);
#endif
        opts.attachment = detail::as_owned_c_ptr(options.attachment);
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);

        __ZENOH_RESULT_CHECK(::z_publisher_put(this->loan(), payload_ptr, &opts), err,
                             "Failed to perform put operation");
    }

    /// @brief Undeclare the resource associated with the publisher key expression.
    /// @param options Optional values to pass to delete operation.
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void delete_resource(DeleteOptions&& options = DeleteOptions::create_default(), ZResult* err = nullptr) const {
        ::z_publisher_delete_options_t opts;
        z_publisher_delete_options_default(&opts);
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);
        __ZENOH_RESULT_CHECK(::z_publisher_delete(this->loan(), &opts), err,
                             "Failed to perform delete_resource operation");
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the publisher.
    /// @note zenoh-c only.
    const KeyExpr& get_keyexpr() const {
        return detail::as_owned_cpp_obj<KeyExpr>(::z_publisher_keyexpr(this->loan()));
    }
#endif
#if defined(ZENOHCXX_ZENOHC) && defined(UNSTABLE)
    /// @brief Get the id of the publisher.
    /// @return id of this publisher.
    EntityGlobalId get_id() const { return EntityGlobalId(::z_publisher_id(this->loan())); }
#endif
};
}  // namespace zenoh
