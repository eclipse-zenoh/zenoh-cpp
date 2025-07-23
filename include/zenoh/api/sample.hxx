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

#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"
#include "timestamp.hxx"
#if defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif

namespace zenoh {
/// @brief A data sample.
///
/// A sample is the value associated to a given resource at a given point in time.
class Sample : public Owned<::z_owned_sample_t> {
    Sample(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Get the resource key of this sample.
    /// @return ``KeyExpr`` object representing the resource key.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_sample_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Get the data of this sample.
    /// @return ``Bytes`` object representing the sample payload.
    const Bytes& get_payload() const {
        return interop::as_owned_cpp_ref<Bytes>(::z_sample_payload(interop::as_loaned_c_ptr(*this)));
    }
#if defined(ZENOHCXX_ZENOHC)
    /// @brief Get the data of this sample.
    /// @return ``Bytes`` object representing the sample payload.
    Bytes& get_payload() {
        return interop::as_owned_cpp_ref<Bytes>(::z_sample_payload_mut(interop::as_loaned_c_ptr(*this)));
    }
#endif

    /// @brief Get the encoding of the data of this sample.
    /// @return ``Encoding`` object.
    const Encoding& get_encoding() const {
        return interop::as_owned_cpp_ref<Encoding>(::z_sample_encoding(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Get the kind of this sample.
    /// @return ``zenoh::SampleKind`` value (PUT or DELETE).
    SampleKind get_kind() const { return ::z_sample_kind(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the attachment of this sample.
    /// @return ``Bytes`` object representing sample attachment.
    std::optional<std::reference_wrapper<const Bytes>> get_attachment() const {
        auto attachment = ::z_sample_attachment(interop::as_loaned_c_ptr(*this));
        if (attachment == nullptr) return {};
        return std::cref(interop::as_owned_cpp_ref<Bytes>(attachment));
    }

    /// @brief Get the timestamp of this sample.
    /// @return ``Timestamp`` object.
    std::optional<Timestamp> get_timestamp() const {
        const ::z_timestamp_t* t = ::z_sample_timestamp(interop::as_loaned_c_ptr(*this));
        if (t == nullptr) {
            return {};
        }
        return interop::as_copyable_cpp_ref<Timestamp>(t);
    }

    /// @brief Get the priority this sample was sent with.
    /// @return ``Priority`` value.
    Priority get_priority() const { return ::z_sample_priority(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the congestion control setting this sample was sent with.
    /// @return ``CongestionControl``  value.
    CongestionControl get_congestion_control() const {
        return ::z_sample_congestion_control(interop::as_loaned_c_ptr(*this));
    }

    /// @brief Get the express setting this sample was sent with.
    /// @return ``CongestionControl`` value.
    bool get_express() const { return ::z_sample_express(interop::as_loaned_c_ptr(*this)); }
#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the source info of this sample.
    const SourceInfo& get_source_info() const {
        return interop::as_owned_cpp_ref<SourceInfo>(::z_sample_source_info(interop::as_loaned_c_ptr(*this)));
    }
#endif

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the reliability this sample was sent with.
    /// @return ``Reliability`` value.
    Reliability reliability() const { return ::z_sample_reliability(interop::as_loaned_c_ptr(*this)); }
#endif

    /// @brief Construct a shallow copy of this sample.
    Sample clone() const {
        Sample s(zenoh::detail::null_object);
        ::z_sample_clone(&s._0, interop::as_loaned_c_ptr(*this));
        return s;
    };
};
}  // namespace zenoh