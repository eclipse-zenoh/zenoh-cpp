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

    /// @brief The resource key of this data sample.
    /// @return ``KeyExpr`` object representing the resource key
    decltype(auto) get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_sample_keyexpr(this->loan())); }

    /// @brief The data of this data sample
    /// @return ``Bytes`` object representing the sample payload
    decltype(auto) get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_sample_payload(this->loan())); }

    /// @brief The encoding of the data of this data sample
    /// @return ``Encoding`` object
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Encoding>(::z_sample_encoding(this->loan())); }

    /// @brief The kind of this data sample (PUT or DELETE)
    /// @return ``zenoh::SampleKind`` value
    SampleKind get_kind() const { return ::z_sample_kind(this->loan()); }

    /// @brief Checks if sample contains an attachment
    /// @return ``True`` if sample contains an attachment
    bool has_attachment() const { return ::z_sample_attachment(this->loan()) != nullptr; }

    /// @brief The attachment of this data sample
    /// @return ``Bytes`` object
    decltype(auto) get_attachment() const { return detail::as_owned_cpp_obj<Bytes>(::z_sample_attachment(this->loan())); }

    /// @brief The timestamp of this data sample
    /// @return ``Timestamp`` object
    decltype(auto) get_timestamp() const { return detail::as_copyable_cpp_obj<Timestamp>(::z_sample_timestamp(this->loan())); }

    /// @brief The priority this data sample was sent with
    /// @return ``Priority``  value
    Priority get_priority() const { return ::z_sample_priority(this->loan()); }

    /// @brief The congestion control setting this data sample was sent with
    /// @return ``CongestionControl``  value
    CongestionControl get_congestion_control() const { return ::z_sample_congestion_control(this->loan()); }

    /// @brief The express setting this data sample was sent with
    /// @return ``CongestionControl``  value
    bool get_express() const { return ::z_sample_express(this->loan()); }

    /// @brief The source info of this sample
    const SourceInfo& get_source_info() const { 
        return detail::as_owned_cpp_obj<SourceInfo>(::z_sample_source_info(this->loan())); 
    }

    /// @brief Constructs a shallow copy of this sample
    Sample clone() const {
        Sample s(nullptr);
        ::z_sample_clone(this->loan(), &s._0);
        return s;
    };
};
}