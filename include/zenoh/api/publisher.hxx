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
#include "bytes.hxx"
#include "encoding.hxx"
#include "keyexpr.hxx"
#include "timestamp.hxx"

#include <optional>


namespace zenoh {
/// An Zenoh publisher. Constructed by ``Session::declare_publisher`` method
class Publisher : public Owned<::z_owned_publisher_t> {
public:
    using Owned::Owned;

    /// @brief Options to be passed to ``Publisher::put()`` operation
    struct PutOptions {
        /// @brief the timestamp of this message
        std::optional<Timestamp> timestamp = {};
        /// @brief The encoding of the data to publish.
        std::optional<Encoding> encoding =  {};
        /// @brief The attachment to attach to the publication.
        std::optional<Bytes> attachment = {};

        /// @brief Returns default option settings
        static PutOptions create_default() { return {}; }
    };

    /// @brief Options to be passed to ``Publisher::delete_resource()`` operation
    struct DeleteOptions {
        /// @brief the timestamp of this message
        std::optional<Timestamp> timestamp = {};

        /// @brief Returns default option settings
        static DeleteOptions create_default() { return {}; }
    };

    /// @name Methods

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @param options Optional values passed to put operation
    /// @return 0 in case of success, negative error code otherwise
    ZError put(Bytes&& payload, PutOptions&& options = PutOptions::create_default()) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_publisher_put_options_t opts;
        z_publisher_put_options_default(&opts);
        opts.encoding = detail::as_owned_c_ptr(options.encoding);
        opts.attachment = detail::as_owned_c_ptr(options.attachment);
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);

        return ::z_publisher_put(this->loan(), payload_ptr, &opts);
    }

    /// @brief Undeclare the resource associated with the publisher key expression
    /// @param options Optional values to pass to delete operation
    /// @return 0 in case of success, negative error code otherwise
    ZError delete_resource(DeleteOptions&& options = DeleteOptions::create_default()) const {
        ::z_publisher_delete_options_t opts;
        z_publisher_delete_options_default(&opts);
        opts.timestamp = detail::as_copyable_c_ptr(options.timestamp);
        return ::z_publisher_delete(this->loan(), &opts);
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the publisher
    decltype(auto) get_keyexpr() const { 
        return detail::as_owned_cpp_obj<KeyExpr>(::z_publisher_keyexpr(this->loan())); 
    }
#endif
};
}
