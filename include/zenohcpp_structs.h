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

#include <iomanip>
#include <iostream>
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
typedef ::z_consolidation_mode_t ConsolidationMode;
typedef ::z_query_target_t QueryTarget;

enum class WhatAmI { Unknown = 0, Router = 1, Peer = 1 << 1, Client = 1 << 2 };

inline const char* as_cstr(WhatAmI whatami) {
    return whatami == WhatAmI::Router   ? "Router"
           : whatami == WhatAmI::Peer   ? "Peer"
           : whatami == WhatAmI::Client ? "Client"
                                        : nullptr;
}

struct StrArray : public Copyable<::z_str_array_t> {
    using Copyable::Copyable;
    const char* operator[](size_t pos) const { return val[pos]; }
    size_t get_len() const { return len; }
};

struct BytesView : public Copyable<::z_bytes_t> {
    using Copyable::Copyable;
    BytesView(void* s, size_t _len) : Copyable({.start = reinterpret_cast<const uint8_t*>(s), .len = _len}) {}
    BytesView(const char* s) : Copyable({.start = reinterpret_cast<const uint8_t*>(s), .len = strlen(s)}) {}
    template <typename T>
    BytesView(const std::vector<T>& v)
        : Copyable({.start = reinterpret_cast<const uint8_t*>(&v[0]), .len = v.size() * sizeof(T)}) {}
    BytesView(const std::string_view& s)
        : Copyable({.start = reinterpret_cast<const uint8_t*>(s.data()), .len = s.length()}) {}
    BytesView(const std::string& s)
        : Copyable({.start = reinterpret_cast<const uint8_t*>(s.data()), .len = s.length()}) {}
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start), len); }
    bool operator==(const BytesView& v) const { return as_string_view() == v.as_string_view(); }
    bool operator!=(const BytesView& v) const { return !operator==(v); }
    size_t get_len() const { return len; }
};

struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
    bool is_some() const { return id[0] != 0; }
};

std::ostream& operator<<(std::ostream& os, const Id& id) {
    for (size_t i = 0; id.id[i] != 0 && i < 16; i++)
        os << std::hex << std::setfill('0') << std::setw(2) << (int)id.id[i];
    return os;
}

struct HelloView : public Copyable<::z_hello_t> {
    using Copyable::Copyable;

    const Id& get_id() const { return static_cast<const Id&>(pid); }
    WhatAmI get_whatami() const {
        return whatami == Z_ROUTER   ? WhatAmI::Router
               : whatami == Z_PEER   ? WhatAmI::Peer
               : whatami == Z_CLIENT ? WhatAmI::Client
                                     : WhatAmI::Unknown;
    }
    const StrArray& get_locators() const { return static_cast<const StrArray&>(locators); }
};

class KeyExpr;

inline bool _split_ret_to_bool_and_err(int8_t ret, ErrNo& error) {
    if (ret < 0) {
        error = ret;
        return false;
    } else {
        error = 0;
        return ret == 1;
    }
}

struct KeyExprView : public Copyable<::z_keyexpr_t> {
    using Copyable::Copyable;
    KeyExprView(nullptr_t) : Copyable(::z_keyexpr(nullptr)) {}  // allow to create uninitialized KeyExprView
    KeyExprView(const char* name) : Copyable(::z_keyexpr(name)) {}
    bool check() const { return ::z_keyexpr_is_initialized(this); }
    BytesView as_bytes() const { return BytesView{::z_keyexpr_as_bytes(*this)}; }
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }

    // operator == between keyexprs purposedly not defided to avoid ambiguity: it's not obvious is string
    // equality or z_keyexpr_equals would be used by operator==
    bool operator==(const std::string_view& v) const { return as_string_view() == v; }
    bool operator!=(const std::string_view& v) const { return !operator==(v); }
    bool equals(const KeyExprView& v) const { return ::z_keyexpr_equals(*this, v) == 1; }

    // operator += purposedly not defined to not provoke ambiguity between concat (which
    // mechanically connects strings) and join (which works with path elements)
    KeyExpr concat(const std::string_view& s) const;
    KeyExpr join(const KeyExprView& v) const;

    bool includes(const KeyExprView& v, ErrNo& error) const {
        return _split_ret_to_bool_and_err(::z_keyexpr_includes(*this, v), error);
    }
    bool includes(const KeyExprView& v) const {
        ErrNo error;
        return includes(v, error);
    }
    bool intersects(const KeyExprView& v, ErrNo& error) const {
        return _split_ret_to_bool_and_err(::z_keyexpr_intersects(*this, v), error);
    }
    bool intersects(const KeyExprView& v) const {
        ErrNo error;
        return includes(v, error);
    }
};

struct Encoding : public Copyable<::z_encoding_t> {
    using Copyable::Copyable;
    Encoding() : Copyable(::z_encoding_default()) {}
    Encoding(EncodingPrefix _prefix) : Copyable(::z_encoding(_prefix, nullptr)) {}
    Encoding(EncodingPrefix _prefix, const char* _suffix) : Copyable(::z_encoding(_prefix, _suffix)) {}

    Encoding& set_prefix(EncodingPrefix _prefix) {
        prefix = _prefix;
        return *this;
    }
    Encoding& set_suffix(const BytesView& _suffix) {
        suffix = _suffix;
        return *this;
    }
    EncodingPrefix get_prefix() const { return prefix; }
    const BytesView& get_suffix() const { return static_cast<const BytesView&>(suffix); }
    bool operator==(const Encoding& v) const {
        return get_prefix() == v.get_prefix() && get_suffix() == v.get_suffix();
    }
    bool operator!=(const Encoding& v) const { return !operator==(v); }
};

struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc
    uint64_t get_time() const { return time; }
    const BytesView& get_id() const { return static_cast<const BytesView&>(id); }
};

struct Sample : public Copyable<::z_sample_t> {
    using Copyable::Copyable;
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const BytesView& get_payload() const { return static_cast<const BytesView&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    SampleKind get_kind() const { return kind; }
};

struct Value : public Copyable<::z_value_t> {
    using Copyable::Copyable;
    Value(const BytesView& payload, const Encoding& encoding) : Copyable({.payload = payload, .encoding = encoding}) {}
    Value(const BytesView& payload) : Value(payload, Encoding()) {}
    Value(const char* payload) : Value(payload, Encoding()) {}

    const BytesView& get_payload() const { return static_cast<const BytesView&>(payload); }
    Value& set_payload(const BytesView& _payload) {
        payload = _payload;
        return *this;
    }

    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    Value& set_encoding(const Encoding& _encoding) {
        encoding = _encoding;
        return *this;
    }

    std::string_view as_string_view() const { return get_payload().as_string_view(); }
    bool operator==(const Value& v) const {
        return get_payload() == v.get_payload() && get_encoding() == v.get_encoding();
    }
    bool operator!=(const Value& v) const { return !operator==(v); }
};

typedef Value ErrorMessage;

struct QueryConsolidation : Copyable<::z_query_consolidation_t> {
    using Copyable::Copyable;
    QueryConsolidation() : Copyable(::z_query_consolidation_default()) {}
    QueryConsolidation(ConsolidationMode v) : Copyable({v}) {}
    QueryConsolidation& set_mode(ConsolidationMode v) {
        mode = v;
        return *this;
    }
    ConsolidationMode get_mode() const { return mode; }
    bool operator==(const QueryConsolidation& v) const { return get_mode() == v.get_mode(); }
    bool operator!=(const QueryConsolidation& v) const { return !operator==(v); }
};

struct GetOptions : public Copyable<::z_get_options_t> {
    using Copyable::Copyable;
    GetOptions() : Copyable(::z_get_options_default()) {}
    GetOptions& set_target(QueryTarget v) {
        target = v;
        return *this;
    }
    GetOptions& set_consolidation(QueryConsolidation v) {
        consolidation = v;
        return *this;
    }
    GetOptions& set_with_value(Value v) {
        with_value = v;
        return *this;
    }
    QueryTarget get_target() const { return target; }
    const QueryConsolidation& get_consolidation() const {
        return static_cast<const QueryConsolidation&>(consolidation);
    }
    const Value& get_with_value() const { return static_cast<const Value&>(with_value); }
    bool operator==(const GetOptions& v) const {
        return get_target() == v.get_target() && get_consolidation() == v.get_consolidation() &&
               get_with_value() == v.get_with_value();
    }
    bool operator!=(const GetOptions& v) const { return !operator==(v); }
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
    BytesView get_parameters() const { return BytesView(::z_query_parameters(this)); }

    bool reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options, ErrNo& error) const {
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options) const {
        ErrNo error;
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const BytesView& payload, ErrNo& error) const {
        return reply_impl(key, payload, nullptr, error);
    }
    bool reply(KeyExprView key, const BytesView& payload) const {
        ErrNo error;
        return reply_impl(key, payload, nullptr, error);
    }

   private:
    bool reply_impl(KeyExprView key, const BytesView& payload, const QueryReplyOptions* options, ErrNo& error) const {
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

struct PublisherDeleteOptions : public Copyable<::z_publisher_delete_options_t> {
    using Copyable::Copyable;
    PublisherDeleteOptions() : Copyable(::z_publisher_delete_options_default()) {}
};

}  // namespace zenoh
