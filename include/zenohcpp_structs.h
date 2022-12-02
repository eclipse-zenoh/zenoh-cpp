//
// Copyright (c) 2022 ZettaScale Technology
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

#include <string>
#include <string_view>

#include "string.h"
#include "zenoh.h"
#include "zenohcpp_base.h"

namespace zenoh {

typedef int8_t ErrNo;
typedef ::z_sample_kind_t SampleKind;
typedef ::z_encoding_prefix_t EncodingPrefix;
typedef ::z_reliability_t Reliability;
typedef ::z_congestion_control_t CongestionControl;
typedef ::z_priority_t Priority;

struct Bytes : public Copyable<::z_bytes_t> {
    using Copyable::Copyable;
    Bytes(const char* s) : Copyable({start : reinterpret_cast<const uint8_t*>(s), len : strlen(s)}) {}
    Bytes(const std::string_view& s)
        : Copyable({start : reinterpret_cast<const uint8_t*>(s.data()), len : s.length()}) {}
    Bytes(const std::string& s) : Copyable({start : reinterpret_cast<const uint8_t*>(s.data()), len : s.length()}) {}
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start), len); }
};

struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
};

struct KeyExprView : public Copyable<::z_keyexpr_t> {
    using Copyable::Copyable;
    KeyExprView(const char* name) : Copyable(z_keyexpr(name)) {}
    bool check() const { return z_keyexpr_is_initialized(this); }
    Bytes as_bytes() const { return Bytes{::z_keyexpr_as_bytes(*this)}; }
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }
};

struct Encoding : public Copyable<::z_encoding_t> {
    using Copyable::Copyable;
    Encoding() : Copyable(::z_encoding_default()) {}
    Encoding(EncodingPrefix _prefix) : Copyable(::z_encoding(_prefix, nullptr)) {}
    Encoding(EncodingPrefix _prefix, const char* _suffix) : Copyable(::z_encoding(_prefix, _suffix)) {}
};

struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc
    uint64_t get_time() const { return time; }
    const Bytes& get_id() const { return static_cast<const Bytes&>(id); }
};

struct Sample : public Copyable<::z_sample_t> {
    using Copyable::Copyable;
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    SampleKind get_kind() const { return kind; }
};

struct Value : public Copyable<::z_value_t> {
    using Copyable::Copyable;
    Value(const char* v) : Copyable({payload : Bytes(v), encoding : Encoding()}) {}
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    std::string_view as_string_view() const { return get_payload().as_string_view(); }
};

typedef Value ErrorMessage;

struct GetOptions : public Copyable<::z_get_options_t> {
    using Copyable::Copyable;
    GetOptions() : Copyable(::z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) {
        target = v;
        return *this;
    }
    GetOptions& set_consolidation(z_consolidation_mode_t v) {
        consolidation.mode = v;
        return *this;
    }
};

struct PutOptions : public Copyable<::z_put_options_t> {
    using Copyable::Copyable;
    PutOptions() : Copyable(::z_put_options_default()) {}
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    PutOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
};

struct DeleteOptions : public Copyable<::z_delete_options_t> {
    using Copyable::Copyable;
    DeleteOptions() : Copyable(::z_delete_options_default()) {}
    CongestionControl get_congestion_control() const { return congestion_control; }
    DeleteOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    }
    Priority get_priority() const { return priority; }
    DeleteOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }
};

struct QueryReplyOptions : public Copyable<::z_query_reply_options_t> {
    using Copyable::Copyable;
    QueryReplyOptions() : Copyable(::z_query_reply_options_default()) {}
    QueryReplyOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
};

class Query : public Copyable<::z_query_t> {
   public:
    using Copyable::Copyable;
    Query() = delete;
    Query(::z_query_t query) : Copyable(query) {}
    KeyExprView get_keyexpr() const { return KeyExprView(::z_query_keyexpr(this)); }
    Bytes get_parameters() const { return Bytes(::z_query_parameters(this)); }

    bool reply(KeyExprView key, const Bytes& payload, const QueryReplyOptions& options, ErrNo& error) const {
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const Bytes& payload, const QueryReplyOptions& options) const {
        ErrNo error;
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const Bytes& payload, ErrNo& error) const {
        return reply_impl(key, payload, nullptr, error);
    }
    bool reply(KeyExprView key, const Bytes& payload) const {
        ErrNo error;
        return reply_impl(key, payload, nullptr, error);
    }

   private:
    bool reply_impl(KeyExprView key, const Bytes& payload, const QueryReplyOptions* options, ErrNo& error) const {
        error = ::z_query_reply(this, key, payload.start, payload.len, options);
        return error == 0;
    }
};

struct QueryableOptions : public Copyable<::z_queryable_options_t> {
    using Copyable::Copyable;
    QueryableOptions() : Copyable(::z_queryable_options_default()) {}
    bool get_complete() const { return complete; }
    QueryableOptions& set_complete(bool v) {
        complete = v;
        return *this;
    }
};

struct SubscriberOptions : public Copyable<::z_subscriber_options_t> {
    using Copyable::Copyable;
    Reliability get_reliability() const { return reliability; }
    SubscriberOptions& set_reliability(Reliability& v) {
        reliability = v;
        return *this;
    }
};

struct PullSubscriberOptions : public Copyable<::z_pull_subscriber_options_t> {
    using Copyable::Copyable;
    Reliability get_reliability() const { return reliability; }
    PullSubscriberOptions& set_reliability(Reliability& v) {
        reliability = v;
        return *this;
    }
};

struct PublisherOptions : public Copyable<::z_publisher_options_t> {
    using Copyable::Copyable;
    PublisherOptions() : Copyable(::z_publisher_options_default()) {}
    CongestionControl get_congestion_control() const { return congestion_control; }
    PublisherOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    }
    Priority get_priority() const { return priority; }
    PublisherOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }
};

struct PublisherPutOptions : public Copyable<::z_publisher_put_options_t> {
    using Copyable::Copyable;
    PublisherPutOptions() : Copyable(::z_publisher_put_options_default()) {}
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    PublisherPutOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
};

}  // namespace zenoh
