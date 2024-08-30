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

#include <functional>

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
    const KeyExpr& get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_sample_keyexpr(this->loan())); }

    /// @brief Get the data of this sample.
    /// @return ``Bytes`` object representing the sample payload.
    const Bytes& get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_sample_payload(this->loan())); }

    /// @brief Get the encoding of the data of this sample.
    /// @return ``Encoding`` object.
    const Encoding& get_encoding() const {
        return detail::as_owned_cpp_obj<Encoding>(::z_sample_encoding(this->loan()));
    }

    /// @brief Get the kind of this sample.
    /// @return ``zenoh::SampleKind`` value (PUT or DELETE).
    SampleKind get_kind() const { return ::z_sample_kind(this->loan()); }

    /// @brief Get the attachment of this sample.
    /// @return ``Bytes`` object representing sample attachment.
    std::optional<std::reference_wrapper<const Bytes>> get_attachment() const {
        auto attachment = ::z_sample_attachment(this->loan());
        if (attachment == nullptr) return {};
        return std::cref(detail::as_owned_cpp_obj<Bytes>(attachment));
    }

    /// @brief Get the timestamp of this sample.
    /// @return ``Timestamp`` object.
    std::optional<Timestamp> get_timestamp() const {
        const ::z_timestamp_t* t = ::z_sample_timestamp(this->loan());
        if (t == nullptr) {
            return {};
        }
        return detail::as_copyable_cpp_obj<Timestamp>(t);
    }

    /// @brief Get the priority this sample was sent with.
    /// @return ``Priority`` value.
    Priority get_priority() const { return ::z_sample_priority(this->loan()); }

    /// @brief Get the congestion control setting this sample was sent with.
    /// @return ``CongestionControl``  value.
    CongestionControl get_congestion_control() const { return ::z_sample_congestion_control(this->loan()); }

    /// @brief Get the express setting this sample was sent with.
    /// @return ``CongestionControl`` value.
    bool get_express() const { return ::z_sample_express(this->loan()); }
#if defined(ZENOHCXX) && defined(UNSTABLE)
    /// @brief Get the source info of this sample.
    const SourceInfo& get_source_info() const {
        return detail::as_owned_cpp_obj<SourceInfo>(::z_sample_source_info(this->loan()));
    }
#endif
    /// @brief Construct a shallow copy of this sample.
    Sample clone() const {
        Sample s(nullptr);
        ::z_sample_clone(&s._0, this->loan());
        return s;
    };
};
}  // namespace zenoh