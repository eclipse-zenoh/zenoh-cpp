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

// Do not add '#pragma once' and '#include` statements here
// as this file is included multiple times into different namespaces

inline QueryTarget query_target_default() { return ::z_query_target_default(); }

inline const char* as_cstr(z::WhatAmI whatami) {
    return whatami == z::WhatAmI::Z_WHATAMI_ROUTER   ? "Router"
           : whatami == z::WhatAmI::Z_WHATAMI_PEER   ? "Peer"
           : whatami == z::WhatAmI::Z_WHATAMI_CLIENT ? "Client"
                                                     : nullptr;
}

void init_logger() {
#ifdef __ZENOHCXX_ZENOHC
    ::zc_init_logger();
#endif
}

inline ::z_bytes_t BytesView::init(const uint8_t* start, size_t len) {
    ::z_bytes_t ret = {.start = start,
                       .len = len
#ifdef __ZENOHCXX_ZENOHPICO
                       ,
                       ._is_alloc = false
#endif
    };
    return ret;
}

struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
    bool is_some() const { return id[0] != 0; }
};

std::ostream& operator<<(std::ostream& os, const z::Id& id) {
    for (size_t i = 0; id.id[i] != 0 && i < 16; i++)
        os << std::hex << std::setfill('0') << std::setw(2) << (int)id.id[i];
    return os;
}

struct HelloView : public Copyable<::z_hello_t> {
    using Copyable::Copyable;

#if defined(__ZENOHCXX_ZENOHC)
    const Id& get_id() const { return static_cast<const Id&>(pid); }
#elif defined(__ZENOHCXX_ZENOHPICO)
    const z::Id& get_id() const {
        assert(zid.len == sizeof(Id));  // TODO: is this invariant that Id is always 16 bytes?
        return reinterpret_cast<const z::Id&>(*zid.start);
    }
#endif
    z::WhatAmI get_whatami() const { return static_cast<z::WhatAmI>(whatami); }
    const z::StrArrayView& get_locators() const { return static_cast<const z::StrArrayView&>(locators); }
};

class KeyExpr;

inline bool _split_ret_to_bool_and_err(int8_t ret, ErrNo& error) {
    if (ret < 0) {
        error = ret;
        return false;
    } else {
        error = 0;
        return ret == 0;
    }
}

inline bool keyexpr_canonize(std::string& s, ErrNo& error) {
    uintptr_t len = s.length();
    error = ::z_keyexpr_canonize(&s[0], &len);
    s.resize(len);
    return error == 0;
}

inline bool keyexpr_canonize(std::string& s) {
    ErrNo error;
    return z::keyexpr_canonize(s, error);
}

inline bool keyexpr_is_canon(const std::string_view& s, ErrNo& error) {
    error = ::z_keyexpr_is_canon(s.begin(), s.length());
    return error == 0;
}

inline bool keyexpr_is_canon(const std::string_view& s) {
    ErrNo error;
    return z::keyexpr_is_canon(s, error);
}

struct KeyExprUnchecked {
    explicit KeyExprUnchecked() {}
};

struct KeyExprView : public Copyable<::z_keyexpr_t> {
    using Copyable::Copyable;
    KeyExprView(nullptr_t) : Copyable(::z_keyexpr(nullptr)) {}  // allow to create uninitialized KeyExprView
    KeyExprView(const char* name) : Copyable(::z_keyexpr(name)) {}
    KeyExprView(const char* name, KeyExprUnchecked) : Copyable(::z_keyexpr_unchecked(name)) {
        assert(keyexpr_is_canon(name));
    }
#ifdef __ZENOHCXX_ZENOHC
    KeyExprView(const std::string_view& name) : Copyable(::zc_keyexpr_from_slice(name.data(), name.length())) {}
    KeyExprView(const std::string_view& name, KeyExprUnchecked)
        : Copyable(::zc_keyexpr_from_slice_unchecked(name.data(), name.length())) {
        assert(keyexpr_is_canon(name));
    }
#endif
    bool check() const { return ::z_keyexpr_is_initialized(this); }
    BytesView as_bytes() const { return BytesView{::z_keyexpr_as_bytes(*this)}; }
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }

    // operator == between keyexprs purposedly not defided to avoid ambiguity: it's not obvious is string
    // equality or z_keyexpr_equals would be used by operator==
    bool operator==(const std::string_view& v) const { return as_string_view() == v; }
    bool operator!=(const std::string_view& v) const { return !operator==(v); }

#ifdef __ZENOHCXX_ZENOHC
    // operator += purposedly not defined to not provoke ambiguity between concat (which
    // mechanically connects strings) and join (which works with path elements)
    KeyExpr concat(const std::string_view& s) const;
    KeyExpr join(const KeyExprView& v) const;
#endif

    bool equals(const KeyExprView& v, ErrNo& error) const {
        return _split_ret_to_bool_and_err(::z_keyexpr_equals(*this, v), error);
    }
    bool equals(const KeyExprView& v) const {
        ErrNo error;
        return equals(v, error);
    }
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
    bool check() const { return ::z_timestamp_check(*this); }
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
    CongestionControl get_congestion_control() const { return congestion_control; }
    PutOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    };
    Priority get_priority() const { return priority; }
    PutOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }
    bool operator==(const PutOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control() &&
               get_encoding() == v.get_encoding();
    }
    bool operator!=(const PutOptions& v) const { return !operator==(v); }
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
    bool operator==(const DeleteOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control();
    }
    bool operator!=(const DeleteOptions& v) const { return !operator==(v); }
};

struct QueryReplyOptions : public Copyable<::z_query_reply_options_t> {
    using Copyable::Copyable;
    QueryReplyOptions() : Copyable(::z_query_reply_options_default()) {}
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    QueryReplyOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
    bool operator==(const QueryReplyOptions& v) const { return get_encoding() == v.get_encoding(); }
    bool operator!=(const QueryReplyOptions& v) const { return !operator==(v); }
};

class Query : public Copyable<::z_query_t> {
   public:
    using Copyable::Copyable;
    KeyExprView get_keyexpr() const { return KeyExprView(::z_query_keyexpr(this)); }
    BytesView get_parameters() const { return BytesView(::z_query_parameters(this)); }
    Value get_value() const { return Value(::z_query_value(this)); }

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
    bool operator==(const QueryableOptions& v) const { return get_complete() == v.get_complete(); }
    bool operator!=(const QueryableOptions& v) const { return !operator==(v); }
};

struct SubscriberOptions : public Copyable<::z_subscriber_options_t> {
    using Copyable::Copyable;
    SubscriberOptions() : Copyable(::z_subscriber_options_default()) {}
    Reliability get_reliability() const { return reliability; }
    SubscriberOptions& set_reliability(Reliability v) {
        reliability = v;
        return *this;
    }
    bool operator==(const SubscriberOptions& v) const { return get_reliability() == v.get_reliability(); }
    bool operator!=(const SubscriberOptions& v) const { return !operator==(v); }
};

struct PullSubscriberOptions : public Copyable<::z_pull_subscriber_options_t> {
    using Copyable::Copyable;
    PullSubscriberOptions() : Copyable(::z_pull_subscriber_options_default()) {}
    Reliability get_reliability() const { return reliability; }
    PullSubscriberOptions& set_reliability(Reliability v) {
        reliability = v;
        return *this;
    }
    bool operator==(const PullSubscriberOptions& v) const { return get_reliability() == v.get_reliability(); }
    bool operator!=(const PullSubscriberOptions& v) const { return !operator==(v); }
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
    bool operator==(const PublisherOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control();
    }
    bool operator!=(const PublisherOptions& v) const { return !operator==(v); }
};

struct PublisherPutOptions : public Copyable<::z_publisher_put_options_t> {
    using Copyable::Copyable;
    PublisherPutOptions() : Copyable(::z_publisher_put_options_default()) {}
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    PublisherPutOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
    bool operator==(const PublisherPutOptions& v) const { return get_encoding() == v.get_encoding(); }
    bool operator!=(const PublisherPutOptions& v) const { return !operator==(v); }
};

struct PublisherDeleteOptions : public Copyable<::z_publisher_delete_options_t> {
    using Copyable::Copyable;
    PublisherDeleteOptions() : Copyable(::z_publisher_delete_options_default()) {}
    bool operator==(const PublisherOptions& v) const { return true; }
    bool operator!=(const PublisherOptions& v) const { return !operator==(v); }
};
