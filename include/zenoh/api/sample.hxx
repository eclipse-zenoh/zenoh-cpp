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
#include "encoding.hxx"
#include "timestamp.hxx"
#include "source_info.hxx"

namespace zenoh {
/// @brief A data sample.
///
/// A sample is the value associated to a given resource at a given point in time.
class Sample : public Owned<::z_owned_sample_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Get the resource key of this sample.
    /// @return ``KeyExpr`` object representing the resource key.
    decltype(auto) get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_sample_keyexpr(this->loan())); }

    /// @brief Get the data of this sample.
    /// @return ``Bytes`` object representing the sample payload.
    decltype(auto) get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_sample_payload(this->loan())); }

    /// @brief Get the encoding of the data of this sample.
    /// @return ``Encoding`` object.
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Encoding>(::z_sample_encoding(this->loan())); }

    /// @brief Get the kind of this sample.
    /// @return ``zenoh::SampleKind`` value (PUT or DELETE).
    SampleKind get_kind() const { return ::z_sample_kind(this->loan()); }

    /// @brief Check if sample contains an attachment.
    /// @return ``true`` if sample contains an attachment, ``false`` otherwise.
    bool has_attachment() const { return ::z_sample_attachment(this->loan()) != nullptr; }

    /// @brief Get the attachment of this sample. Will throw a ZException if ``Sample::has_attachment`` returns ``false``.
    /// @return ``Bytes`` object representing sample attachment.
    decltype(auto) get_attachment() const {
        auto attachment = ::z_sample_attachment(this->loan());
        if (attachment == nullptr) {
            throw ZException("Sample does not contain an attachment", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Bytes>(attachment); 
    }

    /// @brief Get the timestamp of this sample.
    /// @return ``Timestamp`` object.
    decltype(auto) get_timestamp() const { return detail::as_copyable_cpp_obj<Timestamp>(::z_sample_timestamp(this->loan())); }

    /// @brief Get the priority this sample was sent with.
    /// @return ``Priority`` value.
    Priority get_priority() const { return ::z_sample_priority(this->loan()); }

    /// @brief Get the congestion control setting this sample was sent with.
    /// @return ``CongestionControl``  value.
    CongestionControl get_congestion_control() const { return ::z_sample_congestion_control(this->loan()); }

    /// @brief Get the express setting this sample was sent with.
    /// @return ``CongestionControl`` value.
    bool get_express() const { return ::z_sample_express(this->loan()); }
#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the source info of this sample.
    const SourceInfo& get_source_info() const { 
        return detail::as_owned_cpp_obj<SourceInfo>(::z_sample_source_info(this->loan())); 
    }
#endif
    /// @brief Construct a copy of this sample.
    Sample clone() const {
        Sample s(nullptr);
        ::z_sample_clone(&s._0, this->loan());
        return s;
    };
};
}