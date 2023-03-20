//
// Copyright (c) 2023 ZettaScale Technology
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

//
// This file contains structures and classes API without implementations
//

// Do not add '#pragma once' and '#include` statements here
// as this file is included multiple times into different namespaces

// Validate that __ZENOHCXX_ZENOHPICO and __ZENOHCXX_ZENOHC are mutually exclusive
#if defined(__ZENOHCXX_ZENOHPICO) and defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: both __ZENOHCXX_ZENOHC and __ZENOHCXX_ZENOHPICO are defined")
#endif
#if not defined(__ZENOHCXX_ZENOHPICO) and not defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: either __ZENOHCXX_ZENOHC or __ZENOHCXX_ZENOHPICO should be defined")
#endif

//
// Error code value returned as negative value from zenohc/zenohpico functions
//
typedef int8_t ErrNo;

//
// Sample kind values.
//
// Enumerators:
// Z_SAMPLE_KIND_PUT: The Sample was issued by a ``put`` operation.
// Z_SAMPLE_KIND_DELETE: The Sample was issued by a ``delete`` operation.
//
typedef ::z_sample_kind_t SampleKind;

//
// Zenoh encoding values.
// These values are based on already existing HTTP MIME types and extended with other relevant encodings.
//
//  Enumerators:
//      Z_ENCODING_PREFIX_EMPTY: Encoding not defined.
//      Z_ENCODING_PREFIX_APP_OCTET_STREAM: ``application/octet-stream``. Default value for all other cases. An unknown
//          file type should use this type. Z_ENCODING_PREFIX_APP_CUSTOM: Custom application type. Non IANA standard.
//      Z_ENCODING_PREFIX_TEXT_PLAIN: ``text/plain``. Default value for textual files. A textual file should be
//          human-readable and must not contain binary data. Z_ENCODING_PREFIX_APP_PROPERTIES: Application properties
//          type. Non IANA standard. Z_ENCODING_PREFIX_APP_JSON: ``application/json``. JSON format.
//      Z_ENCODING_PREFIX_APP_SQL: Application sql type. Non IANA standard. Z_ENCODING_PREFIX_APP_INTEGER: Application
//          integer type. Non IANA standard. Z_ENCODING_PREFIX_APP_FLOAT: Application float type. Non IANA standard.
//      Z_ENCODING_PREFIX_APP_XML: ``application/xml``. XML.
//      Z_ENCODING_PREFIX_APP_XHTML_XML: ``application/xhtml+xml``. XHTML.
//      Z_ENCODING_PREFIX_APP_X_WWW_FORM_URLENCODED: ``application/x-www-form-urlencoded``. The keys and values are
//          encoded in key-value tuples separated by '&', with a '=' between the key and the value.
//       Z_ENCODING_PREFIX_TEXT_JSON: Text JSON. Non IANA standard. Z_ENCODING_PREFIX_TEXT_HTML: ``text/html``.
//       HyperText
//          Markup Language (HTML). Z_ENCODING_PREFIX_TEXT_XML: ``text/xml``. `Application/xml` is recommended as of RFC
//          7303 (section 4.1), but `text/xml` is still used sometimes. Z_ENCODING_PREFIX_TEXT_CSS: ``text/css``.
//          Cascading Style Sheets (CSS). Z_ENCODING_PREFIX_TEXT_CSV: ``text/csv``. Comma-separated values (CSV).
//      Z_ENCODING_PREFIX_TEXT_JAVASCRIPT: ``text/javascript``. JavaScript.
//      Z_ENCODING_PREFIX_IMAGE_JPEG: ``image/jpeg``. JPEG images.
//      Z_ENCODING_PREFIX_IMAGE_PNG: ``image/png``. Portable Network Graphics.
//      Z_ENCODING_PREFIX_IMAGE_GIF: ``image/gif``. Graphics Interchange Format (GIF).
//
typedef ::z_encoding_prefix_t EncodingPrefix;

//
//  Consolidation mode values.
//
//  Enumerators:
//      Z_CONSOLIDATION_MODE_AUTO: Let Zenoh decide the best consolidation mode depending on the query selector.
//      Z_CONSOLIDATION_MODE_NONE: No consolidation is applied. Replies may come in any order and any number.
//      Z_CONSOLIDATION_MODE_MONOTONIC: It guarantees that any reply for a given key expression will be monotonic in
//      time
//          w.r.t. the previous received replies for the same key expression. I.e., for the same key expression multiple
//          replies may be received. It is guaranteed that two replies received at t1 and t2 will have timestamp
//          ts2 > ts1. It optimizes latency.
//      Z_CONSOLIDATION_MODE_LATEST: It guarantees unicity of replies for the same key expression.
//          It optimizes bandwidth.
//
typedef ::z_consolidation_mode_t ConsolidationMode;

//
// Reliability values.
//
//  Enumerators:
//      Z_RELIABILITY_BEST_EFFORT: Defines reliability as ``BEST_EFFORT``
//      Z_RELIABILITY_RELIABLE: Defines reliability as ``RELIABLE``
//
typedef ::z_reliability_t Reliability;

//
// Congestion control values.
// Enumerators:
//     Z_CONGESTION_CONTROL_BLOCK: Defines congestion control as ``BLOCK``. Messages are not dropped in case of
//         congestion control.
//     Z_CONGESTION_CONTROL_DROP: Defines congestion control as ``DROP``. Messages are dropped in case
//         of congestion control.
//
typedef ::z_congestion_control_t CongestionControl;

//
// Priority of Zenoh messages values.
// Enumerators:
//     _Z_PRIORITY_CONTROL: Priority for ``Control`` messages.
//     Z_PRIORITY_REAL_TIME: Priority for ``RealTime`` messages.
//     Z_PRIORITY_INTERACTIVE_HIGH: Highest priority for ``Interactive`` messages.
//     Z_PRIORITY_INTERACTIVE_LOW: Lowest priority for ``Interactive`` messages.
//     Z_PRIORITY_DATA_HIGH: Highest priority for ``Data`` messages.
//     Z_PRIORITY_DATA: Default priority for ``Data`` messages.
//     Z_PRIORITY_DATA_LOW: Lowest priority for ``Data`` messages.
//     Z_PRIORITY_BACKGROUND: Priority for ``Background traffic`` messages.
//
typedef ::z_priority_t Priority;

//
// Query target values.
// Enumerators:
//     Z_QUERY_TARGET_BEST_MATCHING: The nearest complete queryable if any else all matching queryables.
//     Z_QUERY_TARGET_ALL: All matching queryables.
//     Z_QUERY_TARGET_ALL_COMPLETE: A set of complete queryables.
//
typedef ::z_query_target_t QueryTarget;

//
// Whatami values, defined as a bitmask.
// Enumerators:
//     Z_WHATAMI_ROUTER: Bitmask to filter Zenoh routers.
//     Z_WHATAMI_PEER: Bitmask to filter for Zenoh peers.
//     Z_WHATAMI_CLIENT: Bitmask to filter for Zenoh clients.
//
#ifdef __ZENOHCXX_ZENOHPICO
typedef ::z_whatami_t WhatAmI;
#endif
#ifdef __ZENOHCXX_ZENOHC
enum WhatAmI { Z_WHATAMI_ROUTER = 1, Z_WHATAMI_PEER = 1 << 1, Z_WHATAMI_CLIENT = 1 << 2 };
#endif

//
// Constructs a default QueryTarget
//
inline QueryTarget query_target_default();

// Initializes logger.
// For zenohc set environment variable RUST_LOG=debug or RUST_LOG=info or RUST_LOG=warn or RUST_LOG=error
// to show diagnostic output.
// Does nothing for zenoh-pico at this moment
void init_logger();

//
// StrArrayView represents non-owned read only array of char*
// Impmemented as template to handle with different const definitions in ::z_str_array_t struct in zenohpico and zenohc
//
template <typename Z_STR_ARRAY_T>
struct _StrArrayView : Copyable<Z_STR_ARRAY_T> {
    typedef decltype(Z_STR_ARRAY_T::val) VALTYPE;
    using Copyable<Z_STR_ARRAY_T>::Copyable;
    _StrArrayView() : Copyable<Z_STR_ARRAY_T>({.val = nullptr, .len = 0}) {}
    _StrArrayView(const std::vector<const char*>& v)
        : Copyable<Z_STR_ARRAY_T>({.val = const_cast<VALTYPE>(&v[0]), .len = v.size()}) {}
    _StrArrayView(const char** v, size_t len) : Copyable<Z_STR_ARRAY_T>({.val = const_cast<VALTYPE>(v), .len = len}) {}
    _StrArrayView(const char* const* v, size_t len)
        : Copyable<Z_STR_ARRAY_T>({.val = const_cast<VALTYPE>(v), .len = len}) {}
    const char* operator[](size_t pos) const { return Copyable<Z_STR_ARRAY_T>::val[pos]; }
    size_t get_len() const { return Copyable<Z_STR_ARRAY_T>::len; }
};
struct StrArrayView : z::_StrArrayView<::z_str_array_t> {
    using _StrArrayView<::z_str_array_t>::_StrArrayView;
};

//
// BytesView represents non-owned read only array of bytes
//
class BytesView : public Copyable<::z_bytes_t> {
   public:
    using Copyable::Copyable;
    BytesView(nullptr_t) : Copyable(init(nullptr, 0)) {}
    BytesView(const void* s, size_t _len) : Copyable(init(reinterpret_cast<const uint8_t*>(s), _len)) {}
    BytesView(const char* s) : Copyable({.start = reinterpret_cast<const uint8_t*>(s), .len = s ? strlen(s) : 0}) {}
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
    bool check() const { return ::z_bytes_check(this); }

   private:
    ::z_bytes_t init(const uint8_t* start, size_t len);
};

//
//  Represents a Zenoh ID.
//  In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
//
struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;
    bool is_some() const { return id[0] != 0; }
};
std::ostream& operator<<(std::ostream& os, const z::Id& id);

//
// Represents the non-owned read-only view to a `hello` message returned by a zenoh entity as a reply to a `scout`
// message.
//
struct HelloView : public Copyable<::z_hello_t> {
    using Copyable::Copyable;
    const z::Id& get_id() const;
    z::WhatAmI get_whatami() const { return static_cast<z::WhatAmI>(whatami); }
    const z::StrArrayView& get_locators() const { return static_cast<const z::StrArrayView&>(locators); }
};

class KeyExpr;

//
// Empty type used to distinguish checked and unchecked construncting of KeyExprView
//
struct KeyExprUnchecked {
    explicit KeyExprUnchecked() {}
};

//
// Represents the non-owned read-only view to a key expression in Zenoh.
//
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
    bool equals(const KeyExprView& v, ErrNo& error) const;
    bool equals(const KeyExprView& v) const;
    bool includes(const KeyExprView& v, ErrNo& error) const;
    bool includes(const KeyExprView& v) const;
    bool intersects(const KeyExprView& v, ErrNo& error) const;
    bool intersects(const KeyExprView& v) const;
};

//
// The encoding of a payload, in a MIME-like format.
//
// For wire and matching efficiency, common MIME types are represented using an integer as `prefix`, and a `suffix`
// may be used to either provide more detail, or in combination with the `Empty` prefix to write arbitrary MIME types.
//
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

//
// Represents timestamp value in zenoh
//
struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc
    uint64_t get_time() const { return time; }
    const BytesView& get_id() const { return static_cast<const BytesView&>(id); }
    bool check() const { return ::z_timestamp_check(*this); }
};

//
// A data sample.
//
// A sample is the value associated to a given resource at a given point in time.
//
struct Sample : public Copyable<::z_sample_t> {
    using Copyable::Copyable;
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const BytesView& get_payload() const { return static_cast<const BytesView&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    SampleKind get_kind() const { return kind; }
};

//
// A zenoh value
//
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

//
// Error message returned by some functions
//
typedef Value ErrorMessage;

//
// Represents the replies consolidation to apply on replies of get operation
///
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

//
// Represents the set of options that can be applied to the get operation
//
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

//
// Options passed to the put operation
//
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

//
// Options passed to the delete operation
//
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

//
// Options passed to the query reply
//
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

//
// The query to be answered by a queryable
//
class Query : public Copyable<::z_query_t> {
   public:
    using Copyable::Copyable;
    KeyExprView get_keyexpr() const { return KeyExprView(::z_query_keyexpr(this)); }
    BytesView get_parameters() const { return BytesView(::z_query_parameters(this)); }
    Value get_value() const { return Value(::z_query_value(this)); }

    bool reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options, ErrNo& error) const;
    bool reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options) const;
    bool reply(KeyExprView key, const BytesView& payload, ErrNo& error) const;
    bool reply(KeyExprView key, const BytesView& payload) const;

   private:
    bool reply_impl(KeyExprView key, const BytesView& payload, const QueryReplyOptions* options, ErrNo& error) const;
};

//
// Options to be passed when declaring a queryable
//
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

//
// Options to be passed when declaring a subscriber
//
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

//
// Options to be passed when declaring a pull subscriber
//
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

//
// Options to be passed when declaring a publisher
//
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

//
// Options to be passed to put operation of a publisher
//
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

//
// Options to be passed to delete operation of a publisher
//
struct PublisherDeleteOptions : public Copyable<::z_publisher_delete_options_t> {
    using Copyable::Copyable;
    PublisherDeleteOptions() : Copyable(::z_publisher_delete_options_default()) {}
    bool operator==(const PublisherOptions& v) const { return true; }
    bool operator!=(const PublisherOptions& v) const { return !operator==(v); }
};

//
// Owned string returned from zenoh
//
class Str : public Owned<::z_owned_str_t> {
   public:
    using Owned::Owned;
    operator const char*() const { return ::z_loan(_0); }
    const char* c_str() const { return ::z_loan(_0); }
    bool operator==(const std::string_view& s) const { return s == c_str(); }
    bool operator==(const char* s) const { return std::string_view(s) == c_str(); }
};

//
// Owned key expression
//
class KeyExpr : public Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;
    explicit KeyExpr(nullptr_t) : Owned(nullptr) {}
    explicit KeyExpr(const char* name) : Owned(::z_keyexpr_new(name)) {}
    KeyExprView as_keyexpr_view() const { return KeyExprView(::z_keyexpr_loan(&_0)); }
    operator KeyExprView() const { return as_keyexpr_view(); }
    BytesView as_bytes() const { return as_keyexpr_view().as_bytes(); }
    std::string_view as_string_view() const { return as_keyexpr_view().as_string_view(); }
    bool operator==(const std::string_view& v) { return as_string_view() == v; }
#ifdef __ZENOHCXX_ZENOHC
    KeyExpr concat(const std::string_view& s) const { return as_keyexpr_view().concat(s); }
    KeyExpr join(const KeyExprView& v) const { return as_keyexpr_view().join(v); }
#endif
    bool equals(const KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().equals(v, error); }
    bool equals(const KeyExprView& v) const { return as_keyexpr_view().equals(v); }
    bool includes(const KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().includes(v, error); }
    bool includes(const KeyExprView& v) const { return as_keyexpr_view().includes(v); }
    bool intersects(const KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().intersects(v, error); }
    bool intersects(const KeyExprView& v) const { return as_keyexpr_view().intersects(v); }
};

#ifdef __ZENOHCXX_ZENOHC
KeyExpr KeyExprView::concat(const std::string_view& s) const { return ::z_keyexpr_concat(*this, s.data(), s.length()); }
KeyExpr KeyExprView::join(const KeyExprView& v) const { return ::z_keyexpr_join(*this, v); }
#endif

class ScoutingConfig;

//
// Zenoh config
//
class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
#ifdef __ZENOHCXX_ZENOHC
    Str get(const char* key) const { return Str(::zc_config_get(::z_config_loan(&_0), key)); }
    Str to_string() const { return Str(::zc_config_to_string(::z_config_loan(&_0))); }
    bool insert_json(const char* key, const char* value) {
        return ::zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    const char* get(uint8_t key) const { return ::zp_config_get(::z_config_loan(&_0), key); }
#endif
    ScoutingConfig create_scouting_config();
};

#ifdef __ZENOHCXX_ZENOHC
inline Config config_peer() { return Config(::z_config_peer()); }
std::variant<Config, ErrorMessage> config_from_file(const char* path);
std::variant<Config, ErrorMessage> config_from_str(const char* s);
std::variant<Config, ErrorMessage> config_client(const StrArrayView& peers);
std::variant<Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers);
#endif

//
// Zenoh scouting config
//
class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
    ScoutingConfig(Config& config) : Owned(std::move(ScoutingConfig(config))) {}
};

//
// An owned reply to get operation
//
class Reply : public Owned<::z_owned_reply_t> {
   public:
    using Owned::Owned;
    bool is_ok() const { return ::z_reply_is_ok(&_0); }
    std::variant<Sample, ErrorMessage> get() const {
        if (is_ok()) {
            return Sample{::z_reply_ok(&_0)};
        } else {
            return ErrorMessage{::z_reply_err(&_0)};
        }
    }
};

//
// An owned zenoh subscriber. Destroying subscriber cancels the subscription
//
class Subscriber : public Owned<::z_owned_subscriber_t> {
   public:
    using Owned::Owned;
};

//
// An owned zenoh pull subscriber. Destroying the subscriber cancels the subscription.
//
class PullSubscriber : public Owned<::z_owned_pull_subscriber_t> {
   public:
    using Owned::Owned;
    bool pull() { return ::z_subscriber_pull(::z_loan(_0)) == 0; }
    bool pull(ErrNo& error) {
        error = ::z_subscriber_pull(::z_loan(_0));
        return error == 0;
    }
};

//
// An owned zenoh queryable
//
class Queryable : public Owned<::z_owned_queryable_t> {
   public:
    using Owned::Owned;
};

//
// An owned zenoh publisher
//
class Publisher : public Owned<::z_owned_publisher_t> {
   public:
    using Owned::Owned;
    bool put(const BytesView& payload, const PublisherPutOptions& options, ErrNo& error);
    bool put(const BytesView& payload, ErrNo& error);
    bool put(const BytesView& payload, const PublisherPutOptions& options);
    bool put(const BytesView& payload);
    bool delete_resource(const PublisherDeleteOptions& options, ErrNo& error);
    bool delete_resource(ErrNo& error);
    bool delete_resource(const PublisherDeleteOptions& options);
    bool delete_resource();

   private:
    bool put_impl(const BytesView& payload, const PublisherPutOptions* options, ErrNo& error);
    bool delete_impl(const PublisherDeleteOptions* options, ErrNo& error);
};
