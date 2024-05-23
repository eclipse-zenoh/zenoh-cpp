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

#pragma once

#include "base.hxx"
#include "internal.hxx"
#include <cstddef>
#include <cstdint>

namespace zenoh {


/// ``zenoh::Sample`` kind values.
///
/// Values:
///
///  - **Z_SAMPLE_KIND_PUT**: The Sample was issued by a "put" operation.
///  - **Z_SAMPLE_KIND_DELETE**: The Sample was issued by a "delete" operation.
typedef ::z_sample_kind_t SampleKind;

///   Consolidation mode values.
///
///   Values:
///       - **Z_CONSOLIDATION_MODE_AUTO**: Let Zenoh decide the best consolidation mode depending on the query
///       selector.
///       - **Z_CONSOLIDATION_MODE_NONE**: No consolidation is applied. Replies may come in any order and any
///       number.
///       - **Z_CONSOLIDATION_MODE_MONOTONIC**: It guarantees that any reply for a given key expression will be
///       monotonic in time
///           w.r.t. the previous received replies for the same key expression. I.e., for the same key expression
///           multiple replies may be received. It is guaranteed that two replies received at t1 and t2 will have
///           timestamp ts2 > ts1. It optimizes latency.
///       - **Z_CONSOLIDATION_MODE_LATEST**: It guarantees unicity of replies for the same key expression.
///           It optimizes bandwidth.
typedef ::z_consolidation_mode_t ConsolidationMode;

/// Reliability values.
///
/// Values:
///  - **Z_RELIABILITY_BEST_EFFORT**: Defines reliability as "best effort"
///  - **Z_RELIABILITY_RELIABLE**: Defines reliability as "reliable"
typedef ::z_reliability_t Reliability;

///  Congestion control values.
///
///   Values:
///    - **Z_CONGESTION_CONTROL_BLOCK**: Defines congestion control as "block". Messages are not dropped in case of
///       congestion control
///    - **Z_CONGESTION_CONTROL_DROP**: Defines congestion control as "drop". Messages are dropped in case of
///    congestion control
///
typedef ::z_congestion_control_t CongestionControl;

/// Priority of Zenoh messages values.
///
/// Values:
/// - **Z_PRIORITY_REAL_TIME**: Priority for "realtime" messages.
/// - **Z_PRIORITY_INTERACTIVE_HIGH**: Highest priority for "interactive" messages.
/// - **Z_PRIORITY_INTERACTIVE_LOW**: Lowest priority for "interactive" messages.
/// - **Z_PRIORITY_DATA_HIGH**: Highest priority for "data" messages.
/// - **Z_PRIORITY_DATA**: Default priority for "data" messages.
/// - **Z_PRIORITY_DATA_LOW**: Lowest priority for "data" messages.
/// - **Z_PRIORITY_BACKGROUND**: Priority for "background traffic" messages.
typedef ::z_priority_t Priority;

/// Query target values.
///
/// see also: ``zenoh::query_target_default``
///
/// Values:
/// - **Z_QUERY_TARGET_BEST_MATCHING**: The nearest complete queryable if any else all matching queryables.
/// - **Z_QUERY_TARGET_ALL**: All matching queryables.
/// - **Z_QUERY_TARGET_ALL_COMPLETE**: A set of complete queryables.
typedef ::z_query_target_t QueryTarget;

/// Constructs a default ``zenoh::QueryTarget``
/// @return a default ``zenoh::QueryTarget``
inline QueryTarget query_target_default();

typedef ::z_whatami_t WhatAmI;

/// @brief Returns a string representation of the given ``c::WhatAmI``
/// (or the ``zenohpico::WhatAmI``) value.
/// @param whatami the ``c::WhatAmI`` / ``zenohpico::WhatAmI`` value
/// @return a string representation of the given value
inline std::string_view whatami_as_str(WhatAmI whatami) {
    ::z_view_str_t str_out;
    ::z_whatami_to_str(whatami, &str_out);
    return std::string_view(::z_str_data(::z_loan(str_out)), ::z_str_len(::z_loan(str_out)));
}

#ifdef ZENOHCXX_ZENOHC
/// @brief Initializes logger
///
/// User may set environment variable RUST_LOG to values *debug* | *info* | *warn* | *error* to show diagnostic output
///
/// @note zenoh-c only
void init_logger();
#endif


/// @brief A representation a Zenoh ID.
///
/// In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
///
/// See also: \ref operator_id_out "operator<<(std::ostream& os, const z::Id& id)"
struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;

    /// @name Methods

    /// Checks if the ID is valid
    /// @return true if the ID is valid
    bool is_valid() const { return _0.id[0] != 0; }

    /// Returns the byte sequence of the ID
    const std::array<uint8_t, 16>& bytes() const { return *reinterpret_cast<const std::array<uint8_t, 16>*>(&_0.id); }
};
/// \anchor operator_id_out
///
/// @brief Output operator for Id
/// @param os the output stream
/// @param id reference to the Id
/// @return the output stream
std::ostream& operator<<(std::ostream& os, const Id& id);

/// ``Hello`` message returned by a zenoh entity as a reply to a "scout"
/// message.
class Hello : public Owned<::z_owned_hello_t> {
    using Owned::Owned;

    /// @name Methods

    /// @brief Get ``Id`` of the entity
    /// @return ``Id`` of the entity
    Id get_id() const { return ::z_hello_zid(this->loan()); };

    /// @brief Get the ``zenoh::WhatAmI`` of the entity
    /// @return ``zenoh::WhatAmI`` of the entity
    WhatAmI get_whatami() const { return ::z_hello_whatami(this->loan()); }

    /// @brief Get the array of locators of the entity
    /// @return the array of locators of the entity
    std::vector<std::string_view> get_locators() const {
        ::z_owned_str_array_t out;
        ::z_hello_locators(this->loan(), &out);
        std::vector<std::string_view> locators(::z_str_array_len(::z_loan(out)));
        for (size_t i = 0; i < ::z_str_array_len(::z_loan(out)); i++) {
            auto s = ::z_str_array_get(::z_loan(out), i);
            locators[i] = std::string_view(reinterpret_cast<const char*>(::z_str_data(s)), ::z_str_len(s));
        }
        return locators;
    }
};


/// @brief Owned key expression.
///
/// See details about key expression syntax in the <a
/// href="https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md"> Key Expressions RFC</a>.
class KeyExpr : public Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a new instance from a string
    ///
    /// @param key_expr String representing key expression
    /// @param autocanonize If true the key_expr will be autocanonized, prior to constructing key expression
    /// @param err If not null the error code will be written to this location, otherwise exception will be thrown in case of error.
    explicit KeyExpr(std::string_view key_expr, bool autocanonize = true, ZError* err = nullptr) 
        : Owned(nullptr) {
        if (autocanonize) {
            size_t s = key_expr.size();
            __ZENOH_ERROR_CHECK(
                ::z_keyexpr_from_substring_autocanonize(&this->_0, key_expr.data(), &s), 
                err, 
                std::string("Failed to construct KeyExpr from: ").append(key_expr)
            );
        } else {
            __ZENOH_ERROR_CHECK(
                ::z_keyexpr_from_substring(&this->_0, key_expr.data(), key_expr.size()),
                err,
                std::string("Failed to construct KeyExpr from: ").append(key_expr)
            );
        }
    }

    /// @name Methods
    /// @brief Get underlying key expression string
    std::string_view as_string_view() const {
        ::z_view_str_t s;
        ::z_keyexpr_to_string(this->loan(), &s);
        return std::string_view(reinterpret_cast<const char*>(::z_str_data(::z_loan(s))), ::z_str_len(::z_loan(s)));
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param other the ``std::string_view`` to compare with
    /// @return true if the key expression is equal to the string
    bool operator==(std::string_view other) { return as_string_view() == other; }

    /// @brief Equality operator
    /// @param other the ``KeyExpr`` to compare with
    /// @return true if both key expressions are equal
    bool operator==(const KeyExpr& other) { return ::z_keyexpr_equals(this->loan(), other.loan()); }

    /// @brief Inequality operator
    /// @param other the ``KeyExpr`` to compare with
    /// @return false if both key expressions are equal
    bool operator!=(const KeyExpr& other) { return !(*this == other); }

    /// @brief Inclusion operator
    /// @param other the ``KeyExpr`` to compare with
    /// @return true if other is included in this.
    bool operator>(const KeyExpr& other) {
        return ::z_keyexpr_includes(this->loan(), other.loan());
    }

    /// @brief Constructs new key expression by concatenation this with a string.
    KeyExpr concat(std::string_view s, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_keyexpr_concat(&k._0, this->loan(), s.data(), s.size()),
            err,
            std::string("Failed to concatenate KeyExpr: ").append(this->as_string_view()).append(" with ").append(s)
        );
        return k;
    }

    /// @brief Constructs new key expression by joining this with another one
    KeyExpr join(const KeyExpr& other, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_keyexpr_join(&k._0, this->loan(), other.loan()),
            err,
            std::string("Failed to join KeyExpr: ").append(this->as_string_view()).append(" with ").append(other.as_string_view())
        );
        return k;
    }

    /// @brief Checks if 2 key expressions intersect
    /// @return true if there is at least one non-empty key that is contained in both key expressions
    bool intersects(const KeyExpr& other) const { return ::z_keyexpr_intersects(this->loan(), other.loan()); }

    typedef ::z_keyexpr_intersection_level_t IntersectionLevel;

    IntersectionLevel relation_to(const KeyExpr& other) { return ::z_keyexpr_relation_to(this->loan(), other.loan()); }

    /// @brief Verifies if the string is a canonical key expression
    static bool is_canon(std::string_view s) {
        return ::z_keyexpr_is_canon(s.data(), s.size()) == Z_OK;
    }
};

class Encoding : public Owned<::z_owned_encoding_t> {
    using Owned::Owned;

    /// @name Constructors

    /// @brief Default encoding
    Encoding() : Owned(nullptr) {}

    /// @brief Constructs encoding from string
    Encoding(std::string_view s, ZError* err = nullptr) : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(
            ::z_encoding_from_substring(&this->_0, s.data(), s.size()),
            err,
            std::string("Failed to create encoding from ").append(s)
        );
    }

    /// @brief Converts encoding to a string
    std::string as_string() const {
        ::z_owned_str_t s;
        ::z_encoding_to_string(this->loan(), &s);
        std::string out = std::string(::z_str_data(::z_loan(s)), ::z_str_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }
};

/// Zenoh <a href=https://zenoh.io/docs/manual/abstractions/#timestamp>Timestamp</a>.
struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc

    /// @name Methods

    /// @brief Get the NPT64 time part of timestamp
    /// @return time in NPT64 format.
    uint64_t get_time() const { return ::z_timestamp_npt64_time(&this->inner()); }

    /// @brief Get the unique id of the timestamp
    /// @return unique id
    Id get_id() const { return ::z_timestamp_id(&this->inner()); }
};

class Bytes;

namespace serde {
    template<class T>
    Bytes serialize(T data);

    template<class T>
    T deserialize(const Bytes& b, ZError* err = nullptr);
}

/// @brief A Zenoh serialized data representation
class Bytes : public Owned<::z_owned_bytes_t> {
public:
    using Owned::Owned;

    /// @name Methods
     

    /// @brief Constructs a shallow copy of this data
    Bytes clone() const {
        Bytes b(nullptr);
        ::z_bytes_clone(this->loan(), &b._0);
        return b; 
    }

    /// @brief Get number of bytes in the pyload.
    size_t size() const {
        return ::z_bytes_len(this->loan());
    }

    /// @brief Serialize specified type.
    ///
    /// @tparam T Type to serialize
    /// @param data Instance of T to serialize
    template<class T>
    Bytes serialize(T data) const {
        return zenoh::serde::serialize(data);
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T Type to deserialize into
    template<class T>
    T deserialize(ZError* err) const {
        return zenoh::serde::deserialize<T>(*this, err);
    }

    class BytesReader : public Owned<::z_owned_bytes_reader_t> {
    public:
        using Owned::Owned;

        size_t read(uint8_t* buf, size_t len) {
            return ::z_bytes_reader_read(this->loan(), buf, len);
        }

        int64_t position() {
            return ::z_bytes_reader_tell(this->loan());
        }


        void seek_from_current(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(this->loan(), offset, SEEK_CUR),
                err,
                "seek_from_current failed"
            );
        }

        void seek_from_start(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(this->loan(), offset, SEEK_SET),
                err,
                "seek_from_start failed"
            );
        }

        void seek_from_end(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(this->loan(), offset, SEEK_END),
                err,
                "seek_from_end failed"
            );
        }
    };

    /// @brief Create data reader
    /// @return 
    BytesReader reader() const {
        ::z_owned_bytes_reader_t br;
        ::z_bytes_reader_new(&br, this->loan());
        return BytesReader(&br);
    }
};




/// @brief A data sample.
///
/// A sample is the value associated to a given resource at a given point in time.
class Sample : public Owned<::z_owned_sample_t> {
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
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Sample>(::z_sample_encoding(this->loan())); }

    /// @brief The kind of this data sample (PUT or DELETE)
    /// @return ``zenoh::SampleKind`` value
    SampleKind get_kind() const { return ::z_sample_kind(this->loan()); }

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

    /// @brief Constructs a shallow copy of this Sample
    Sample clone() const {
        Sample s(nullptr);
        ::z_sample_clone(this->loan(), &s._0);
        return s;
    };
};

/// A zenoh value. Contans refrence to data and it's encoding
class Value : public Owned<::z_owned_value_t> {
    using Owned::Owned;
    /// @name Methods

    /// @brief The payload of this value
    /// @return ``Bytes`` object
    decltype(auto) get_payload() const { return detail::as_owned_cpp_obj<Bytes>(::z_value_payload(this->loan())); }

    /// @brief The encoding of this value
    /// @return ``Encoding`` object
    decltype(auto) get_encoding() const { return detail::as_owned_cpp_obj<Encoding>(::z_value_encoding(this->loan())); }
};

/// Replies consolidation mode to apply on replies of get operation
struct QueryConsolidation : Copyable<::z_query_consolidation_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``QueryConsolidation`` value
    QueryConsolidation() : Copyable(::z_query_consolidation_default()) {}

    /// @brief Create a new ``QueryConsolidation`` value with the given consolidation mode
    /// @param v ``zenoh::ConsolidationMode`` value
    QueryConsolidation(ConsolidationMode v) : Copyable({v}) {}

    /// @name Methods

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``QueryConsolidation`` to compare with
    /// @return true if the two values are equal (have the same consolidation mode)
    bool operator==(const QueryConsolidation& v) const { return this->_0.mode == v._0.mode; }

    /// @brief Inequality operator
    /// @param v the other ``QueryConsolidation`` to compare with
    /// @return true if the two values are not equal (have different consolidation mode)
    bool operator!=(const QueryConsolidation& v) const { return !operator==(v); }
};

/// The query to be answered by a ``Queryable``
class Query : public Owned<::z_owned_query_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Get the key expression of the query
    /// @return ``KeyExpr`` value
    decltype(auto) get_keyexpr() const { return detail::as_owned_cpp_obj<KeyExpr>(::z_query_keyexpr(this->loan())); }

    /// @brief Get a query's <a href=https://github.com/eclipse-zenoh/roadmap/tree/main/rfcs/ALL/Selectors>parameters</a>
    ///
    /// @return Parameters string
    std::string_view get_parameters() const {
        ::z_view_str_t p;
        ::z_query_parameters(this->loan(), &p);
        return std::string_view(::z_str_data(::z_loan(p)), ::z_str_len(::z_loan(p))); 
    }

    /// @brief Get the value of the query (payload and encoding)
    /// @return ``Value`` value
    decltype(auto) get_value() const { return detail::as_owned_cpp_obj<Value>(::z_query_value(this->loan())); }

    /// @brief Get the attachment of the query
    /// @return Attachment
    decltype(auto) get_attachment() const { return detail::as_owned_cpp_obj<Bytes>(::z_query_attachment(this->loan())); }

    /// Options passed to the ``Query::reply`` operation
    struct ReplyOptions {
        /// @brief An optional encoding of this reply payload and/or attachment
        Encoding encoding = Encoding(nullptr);
        /// @brief An optional attachment to this reply.
        Bytes attachment = Bytes(nullptr);
    };

    /// @brief Send reply to a query
    ::z_error_t reply(const KeyExpr& key_expr, Bytes&& payload, ReplyOptions&& options = {}) const {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_options_t opts = {
            .attachment = detail::as_owned_c_ptr(options.attachment),
            .encoding = detail::as_owned_c_ptr(options.encoding)
        };

        return ::z_query_reply(
            this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), payload_ptr, &opts
        );
    }

    /// Options passed to the ``Query::reply_err`` operation
    struct ReplyErrOptions {
        /// @brief An optional encoding of the reply error payload
        Encoding encoding = Encoding(nullptr);
    };

    /// @brief Send error to a query
    ::z_error_t reply_err(const KeyExpr& key_expr, Bytes&& payload, ReplyErrOptions&& options = {}) {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_query_reply_err_options_t opts = {
            .encoding = detail::as_owned_c_ptr(options.encoding)
        };

        return ::z_query_reply_err(
            this->loan(), payload_ptr, &opts
        );
    }
};


/// A Zenoh Session config
class Config : public Owned<::z_owned_config_t> {
public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a default configuration
    Config() : Owned(nullptr) {
        ::z_config_default(&this->_0);
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get config parameter by the string key
    /// @param key the key
    /// @return value of the config parameter
    /// @note zenoh-c only
    std::string get(std::string_view key, ZError* err = nullptr) const {
        ::z_owned_str_t s;
        __ZENOH_ERROR_CHECK(
            ::zc_config_get_from_substring(this->loan(), key.data(), key.size(), &s),
            err,
            std::string("Failed to get config value for the key: ").append(key)
        );
        std::string out = std::string(::z_str_data(::z_loan(s)), ::z_str_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Get the whole config as a JSON string
    /// @return the JSON string in ``Str``
    /// @note zenoh-c only
    std::string to_string() const { 
        ::z_owned_str_t s;
        ::zc_config_to_string(this->loan(), &s);
        std::string out = std::string(::z_str_data(::z_loan(s)), ::z_str_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out; 
    }

    /// @brief Insert a config parameter by the string key
    /// @param key the key
    /// @param value the JSON string value
    /// @return true if the parameter was inserted
    /// @note zenoh-c only
    bool insert_json(const std::string& key, const std::string& value) { 
        return ::zc_config_insert_json(loan(), key.c_str(), value.c_str()) == 0; 
    }
#endif
#ifdef ZENOHCXX_ZENOHPICO
    /// @brief Get config parameter by it's numeric ID
    /// @param key the key
    /// @return pointer to the null-terminated string value of the config parameter
    /// @note zenoh-pico only
    const char* get(uint8_t key) const { return ::zp_config_get(loan(), key); }

    /// @brief Insert a config parameter by it's numeric ID
    /// @param key the key
    /// @param value the null-terminated string value
    /// @return true if the parameter was inserted
    /// @note zenoh-pico only
    bool insert(uint8_t key, const char* value);

    /// @brief Insert a config parameter by it's numeric ID
    /// @param key the key
    /// @param value the null-terminated string value
    /// @param error the error code
    /// @return true if the parameter was inserted
    /// @note zenoh-pico only
    bool insert(uint8_t key, const char* value, ErrNo& error);
#endif

#ifdef ZENOHCXX_ZENOHC
    /// @brief Create the default configuration for "peer" mode
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config peer() {
        Config c(nullptr) ;
        ::z_config_peer(&c._0);
        return c;
    }

    /// @brief Create the configuration from the JSON file
    /// @param path path to the file
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config from_file(const std::string& path, ZError* err = nullptr) {
        Config c(nullptr) ;
        __ZENOH_ERROR_CHECK(
            ::zc_config_from_file(&c._0, path.data()),
            err,
            std::string("Failed to create config from: ").append(path)
        );
        return c;
    }

    /// @brief Create the configuration from the JSON string
    /// @param s the JSON string
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config from_str(const std::string& s, ZError* err = nullptr) {
        Config c(nullptr) ;
        __ZENOH_ERROR_CHECK(
            ::zc_config_from_str(&c._0, s.data()),
            err,
            std::string("Failed to create config from: ").append(s)
        );
        return c;
    }
    /// @brief Create the configuration for "client" mode
    /// @param peers the array of peer endpoints
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config client(const std::vector<std::string>& peers, ZError* err = nullptr) {
        Config c(nullptr) ;
        std::vector<const char*> p;
        p.reserve(peers.size());
        for (const auto& peer: peers) {
            p.push_back(peer.c_str());
        }
        __ZENOH_ERROR_CHECK(
            ::z_config_client(&c._0, p.data(), p.size()),
            err,
            "Failed to create client config"
        );
        return c;
    }
    #endif
};



/// A reply from queryable to ``Session::get`` operation
class Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Check if the reply is OK
    /// @return true if the reply is OK, false if contains a error
    bool is_ok() const { return ::z_reply_is_ok(this->loan()); }

    /// @brief Get the reply value. Will throw an exception if ``Reply::is_ok`` returns false.
    /// @return Reply sample.
    decltype(auto) get_ok() const {
        if (!::z_reply_is_ok(this->loan())) {
            throw ZException("Reply data sample was requested, but reply contains error", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Sample>(::z_reply_ok(this->loan()));
    }

    /// @brief Get the reply error. Will throw an exception if ``Reply::is_ok`` returns false.
    /// @return Reply error.
    decltype(auto) get_err() const {
        if (::z_reply_is_ok(this->loan())) {
            throw ZException("Reply error was requested, but reply contains data sample", Z_EINVAL);
        }
        return detail::as_owned_cpp_obj<Value>(::z_reply_err(this->loan()));
    }
};

/// An Zenoh subscriber. Destroying subscriber cancels the subscription
/// Constructed by ``Session::declare_subscriber`` method
class Subscriber : public Owned<::z_owned_subscriber_t> {
public:
    using Owned::Owned;

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the subscriber
    decltype(auto) get_keyexpr() const { 
        return detail::as_owned_cpp_obj<KeyExpr>(::z_subscriber_keyexpr(this->loan())); 
    }
#endif
};

/// An Zenoh queryable. Constructed by ``Session::declare_queryable`` method
class Queryable : public Owned<::z_owned_queryable_t> {
public:
    using Owned::Owned;
};

/// An Zenoh publisher. Constructed by ``Session::declare_publisher`` method
class Publisher : public Owned<::z_owned_publisher_t> {
public:
    using Owned::Owned;

    /// Options to be passed to ``Publisher::put`` operation
    struct PutOptions {
        /// @brief The encoding of the data to publish.
        Encoding encoding = Encoding(nullptr);
        /// @brief The attachment to attach to the publication.
        Bytes attachment = Bytes(nullptr);
    };

    /// Options to be passed to delete operation of a publisher
    struct DeleteOptions {
        uint8_t __dummy;
    };

    /// @name Methods

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @param options Optional values passed to put operation
    /// @return 0 in case of success, negative error code otherwise
    ZError put(Bytes&& payload, PutOptions&& options = {}) {
        auto payload_ptr = detail::as_owned_c_ptr(payload);
        ::z_publisher_put_options_t opts = {
            .attachment = detail::as_owned_c_ptr(options.attachment),
            .encoding = detail::as_owned_c_ptr(options.encoding)
        };
        return ::z_publisher_put(this->loan(), payload_ptr, &opts);
    }

    /// @brief Undeclare the resource associated with the publisher key expression
    /// @param options Optional values to pass to delete operation
    /// @return 0 in case of success, negative error code otherwise
    ZError delete_resource(DeleteOptions&& options = {}) {
        ::z_publisher_delete_options_t opts {
            .__dummy = 0
        };
        return ::z_publisher_delete(this->loan(), &opts);
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the publisher
    decltype(auto) get_keyexpr() const { 
        return detail::as_owned_cpp_obj<KeyExpr>(::z_publisher_keyexpr(this->loan())); 
    }
#endif
};

namespace detail {
template<class F>
struct ReplyClosure {
    static void call(const ::z_loaned_reply_t* reply, void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Reply&>*>(context);
        (*f)(detail::as_owned_cpp_obj<Reply>(reply));
    }

    static void drop(void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Reply&>*>(context);
        delete f;
    }
};

template<class F>
struct QueryClosure {
    static void call(const ::z_loaned_query_t* query, void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Query&>*>(context);
        (*f)(detail::as_owned_cpp_obj<Query>(query));
    }

    static void drop(void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Query&>*>(context);
        delete f;
    }
};

template<class F>
struct SampleClosure {
    static void all(const ::z_loaned_sample_t* sample, void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Sample&>*>(context);
        (*f)(detail::as_owned_cpp_obj<Sample>(sample));
    }

    static void drop(void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Sample&>*>(context);
        delete f;
    }
};

template<class F>
struct ZIdClosure {
    static void call(const ::z_id_t* z_id, void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Id&>*>(context);
        (*f)(detail::as_copyable_cpp_obj<Id>(z_id));
    }

    static void drop(void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Id&>*>(context);
        delete f;
    }
};

template<class F>
struct HelloClosure {
    static void all(const ::z_loaned_sample_t* hello, void* context) {
        auto f = reinterpret_cast<Closure<F, void, const Hello&>*>(context);
        (*f)(detail::as_copyable_cpp_obj<Sample>(hello));
    }

    static void drop(void* context) {
        auto f = reinterpret_cast<Closure<Hello, void, const Hello&>*>(context);
        delete f;
    }
};

}

enum class FifoChannelType {
    Blocking,
    NonBlocking
};


template<FifoChannelType C>
class ReplyFifoChannel: public Owned<::z_owned_reply_channel_closure_t> {
private:
    bool active = true;
public:
    using Owned::Owned;
    /// @brief Fetches next available reply.
    /// @return Reply. Might return invalid reply in case of Non-blocking channel if there are no replies available.
    /// In this case is is possible to verify whether the channel is still active (and might receive more replies in the future)
    /// by calling  ``ReplyFifoChannel::is_active()``.
    Reply next_reply() {
        Reply r(nullptr);
        res = ::z_call(this->_0, detail::as_owned_c_ptr(r));
        if constexpr (C == FifoChannelType::NonBlocking) {
            active = res;
        } else {
            active = static_cast<bool>(r);
        }
        return res;
    }

    /// @brief Verifies if channel is still active.
    /// @return True if channel is still active (i. e. might receive more valid replies in the future). Inactive channel will
    /// always return an invalid ``Reply`` when calling  ``ReplyFifoChannel::next_reply()`.
    bool is_active() const { return active; }
};

/// A Zenoh session.
class Session : public Owned<::z_owned_session_t> {
public:
    using Owned::Owned;

    Session(Config&& config, ZError* err = nullptr) : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(
            ::z_open(&this->_0, detail::as_owned_c_ptr(config)),
            err,
            "Failed to open session"
        );
    }

    /// @name Methods

#ifdef ZENOHCXX_ZENOHC
    /// @brief Create a shallow copy of the session
    /// @return a new ``Session`` instance
    /// @note zenoh-c only
    Session clone() const {
        Session s(nullptr);
        ::zc_session_clone(this->loan(), &s._0);
        return s;
    }
#endif

    /// @brief Get the unique identifier of the zenoh node associated to this ``Session``
    /// @return the unique identifier ``Id``
    Id get_zid() const { 
         return Id(::z_info_zid(this->loan())); 
    }

    /// @brief Create ``KeyExpr`` instance with numeric id registered in ``Session`` routing tables
    /// @param key_expr ``KeyExpr`` to declare
    /// @return Declared ``KeyExpr`` instance
    KeyExpr declare_keyexpr(const KeyExpr& key_expr, ZError* err = nullptr) const {
        KeyExpr k(nullptr);
        __ZENOH_ERROR_CHECK(
            ::z_declare_keyexpr(detail::as_owned_c_ptr(k), this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr)),
            err,
            std::string("Failed to declare key expression: ").append(k.as_string_view())
        );
        return k;
    }

    /// @brief Remove ``KeyExpr`` instance from ``Session`` and drop ``KeyExpr`` instance
    /// @param keyexpr ``KeyExpr`` instance to drop
    /// @return 0 on success, negative error code otherwise.
    ZError undeclare_keyexpr(KeyExpr&& key_expr) const {
        return ::z_undeclare_keyexpr(detail::as_owned_c_ptr(key_expr), this->loan());
    }

    /// Options passed to the get operation
    struct GetOptions {
        /// @brief The Queryables that should be target of the query.
        QueryTarget target = QueryTarget::Z_QUERY_TARGET_ALL;
        /// @brief The replies consolidation strategy to apply on replies to the query.
        QueryConsolidation consolidation = QueryConsolidation();
        /// @brief An optional payload of the query.
        Bytes payload = Bytes(nullptr);
        /// @brief  An optional encoding of the query payload and/or attachment.
        Encoding encoding = Encoding(nullptr);
        /// An optional attachment to the query.
        Bytes attachment = Bytes(nullptr);
        /// @brief The timeout for the query in milliseconds. 0 means default query timeout from zenoh configuration.
        uint64_t timeout_ms = 0;
    };

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param key_expr ``KeyExpr`` the key expression matching resources to query
    /// @param parameters the parameters string in URL format
    /// @param callback ``zenoh::ClosureReply`` callback to process ``Reply``s
    /// @param options ``GetOptions`` query options
    /// @return 0 in case of success, negative error code otherwise
    template<class F>
    ZError get(const KeyExpr& key_expr, const std::string& parameters, F&& callback, GetOptions&& options = {}) const {
        static_assert(
            std::is_invocable_r<F, void, const Reply&>::value,
            "Callback should be callable with the following signature void callback(const zenoh::Reply& reply)"
        );
        ::z_owned_closure_reply_t c_closure;
        auto closure = new Closure<F, void, const Reply&>(std::forward<F>(callback));
        ::z_closure(&c_closure, &detail::ReplyClosure<F>::call, &detail::ReplyClosure<F>::drop, reinterpret_cast<void*>(closure));
        ::z_get_options_t opts = {
            .target = options.target,
            .consolidation = static_cast<const z_query_consolidation_t&>(options.consolidation),
            .encoding = detail::as_owned_c_ptr(options.encoding),
            .payload = detail::as_owned_c_ptr(options.payload),
            .attachment = detail::as_owned_c_ptr(options.attachment),
            .timeout = options.timeout
        };
        return ::z_get(this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), parameters.c_str(), ::z_move(c_closure), &opts);
    }

    /// @brief Query data from the matching queryables in the system. Replies are provided through a channel.
    /// @param key_expr ``KeyExpr`` the key expression matching resources to query
    /// @param parameters the parameters string in URL format
    /// @param bound Capacity of the channel, if different from 0, the channel will be bound and apply back-pressure when full
    /// @param options ``GetOptions`` query options
    /// @return Reply fifo channel
    template<FifoChannelType C>
    ReplyFifoChannel<C> get_reply_fifo_channel(const KeyExpr& key_expr, const std::string& parameters, size_t bound, GetOptions&& options = {}, ZError* err = nullptr) const {
        ::z_owned_reply_channel_t reply_channel;
        if constexpr (C == FifoChannelType::Blocking) {
            ::zc_reply_fifo_new(&reply_channel, bound);
        } else {
            ::zc_reply_non_blocking_fifo_new(&reply_channel, bound);
        }
        ReplyFifoChannel recv(&reply_channel.recv);
        ::z_get_options_t opts = {
            .target = options.target,
            .consolidation = static_cast<const z_query_consolidation_t&>(options.consolidation),
            .encoding = detail::as_owned_c_ptr(options.encoding),
            .payload = detail::as_owned_c_ptr(options.payload),
            .attachment = detail::as_owned_c_ptr(options.attachment),
            .timeout_ms = options.timeout_ms
        };
        ZError res = ::z_get(this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), parameters.c_str(), ::z_move(reply_channel.send), &opts);
        __ZENOH_ERROR_CHECK(
            res,
            err,
            "Failed to perform get operation"
        );
        if (res != Z_OK) std::move(recv).take();
        return recv;
    }
    /// Options to be passed to delete operation
    struct DeleteOptions {
        /// @brief The priority of the delete message.
        Priority priority = Priority::Z_PRIORITY_DATA;
        /// @brief The congestion control to apply when routing delete message.
        CongestionControl congestion_control = CongestionControl::Z_CONGESTION_CONTROL_DROP;
        /// @brief Whether Zenoh will NOT wait to batch delete message with others to reduce the bandwith.
        bool is_express = false;
    };

    /// @brief Undeclare a resource. Equal to ``Publisher::delete_resource``
    /// @param key_expr ``KeyExprView`` the key expression to delete the resource
    /// @param options ``DeleteOptions`` delete options
    /// @return 0 in case of success, negative error code otherwise
    ZError delete_resource(const KeyExpr& key_expr, DeleteOptions&& options = {}) const {
        ::z_delete_options_t opts = {
            .priority = options.priority,
            .congestion_control = options.congestion_control,
            .is_express = options.is_express
        };
        return ::z_delete(this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), &opts);
    }

    /// Options passed to the put operation
    struct PutOptions {
        /// @brief The priority of this message.
        Priority priority = Priority::Z_PRIORITY_DATA;
        /// @brief The congestion control to apply when routing this message.
        CongestionControl congestion_control = CongestionControl::Z_CONGESTION_CONTROL_DROP;
        /// @brief Whether Zenoh will NOT wait to batch this message with others to reduce the bandwith.
        bool is_express = false;
        /// @brief  An optional encoding of the message payload and/or attachment.
        Encoding encoding = Encoding(nullptr);
        /// An optional attachment to the message.
        Bytes attachment = Bytes(nullptr);
    };

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param key_expr The key expression to put the data
    /// @param payload The data to publish
    /// @param options Options to pass to put operation
    /// @return 0 in case of success, negative error code otherwise
    ZError put(const KeyExpr& key_expr, Bytes&& payload, PutOptions&& options = {}) const {
        ::z_put_options_t opts = {
            .priority = options.priority,
            .congestion_control = options.congestion_control,
            .is_express = options.is_express,
            .encoding = detail::as_owned_c_ptr(options.encoding),
            .attachment = detail::as_owned_c_ptr(options.attachment)
        };

        auto payload_ptr = detail::as_owned_c_ptr(payload);
        return ::z_put(this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), payload_ptr, &opts);
    }

    /// Options to be passed when declaring a ``Queryable``
    struct QueryableOptions {
        /// @brief The completeness of the Queryable.
        bool complete = false;
    };

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests
    /// @param key_expr The key expression to match the ``Session::get`` requests
    /// @param callback The callback to handle ``Query`` requests
    /// @param options Options passed to queryable declaration
    /// @return a ``Queryable`` object
    template<class F>
    Queryable declare_queryable(const KeyExpr& key_expr, F&& callback, QueryableOptions&& options = {}, ZError* err = nullptr) const {
        static_assert(
            std::is_invocable_r<F, void, const Query&>::value,
            "Callback should be callable with the following signature void callback(const zenoh::Query& query)"
        );
        ::z_owned_closure_query_t c_closure;
        auto closure = new Closure<F, void, const Query&>(std::forward<F>(callback));
        ::z_closure(&c_closure, &detail::QueryClosure<F>::call, &detail::QueryClosure<F>::drop, reinterpret_cast<void*>(closure));
        ::z_queryable_options_t opts = {
            .complete = options.complete
        };
        Queryable q(nullptr);
        ZError res =  ::z_declare_queryable(
            detail::as_owned_c_ptr(q), this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), ::z_move(c_closure), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Queryable");
        return q;
    }

    struct SubscriberOptions {
        /// @brief The subscription reliability.
        Reliability reliability = Reliability::Z_RELIABILITY_BEST_EFFORT;
    };

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put`` and ``Session::delete_resource`` requests
    /// @param key_expr The key expression to match the publishers
    /// @param callback The callback to handle the received ``Sample`` objects
    /// @param options Options to pass to subscriber declaration
    /// @return a ``Subscriber`` object
    template<class F>
    Subscriber declare_subscriber(const KeyExpr& key_expr, F&& callback, SubscriberOptions&& options = {}, ZError *err = nullptr) const {
        static_assert(
            std::is_invocable_r<F, void, const Sample&>::value,
            "Callback should be callable with the following signature void callback(const zenoh::Sample& sample)"
        );
        ::z_owned_closure_query_t c_closure;
        auto closure = new Closure<F, void, const Sample&>(std::forward<F>(callback));
        ::z_closure(&c_closure, &detail::SampleClosure<F>::call, &detail::SampleClosure<F>::drop, reinterpret_cast<void*>(closure));
        ::z_subscriber_options_t opts = {
            .reliability = options.complete
        };
        Subscriber s(nullptr);
        ZError res =  ::z_declare_subscriber(
            detail::as_owned_c_ptr(s), this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), ::z_move(c_closure), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Subscriber");
        return s;
    }

    struct PublisherOptions {
        /// @brief The congestion control to apply when routing messages from this publisher.
        CongestionControl congestion_control;
        /// @brief The priority of messages from this publisher.
        Priority priority;
        /// @brief If true, Zenoh will not wait to batch this message with others to reduce the bandwith
        bool is_express;
    };

    /// @brief Create a ``Publisher`` object to publish data to matching ``Subscriber`` and ``PullSubscriber`` objects
    /// @param key_expr The key expression to match the subscribers
    /// @param options Options passed to publisher declaration
    /// @return a ``Publisher`` object
    Publisher declare_publisher(const KeyExpr& key_expr, PublisherOptions&& options, ZError* err = nullptr) const {
        ::z_publisher_options_t opts = {
            .congestion_control = options.congestion_control,
            .priority = options.priority,
            .is_express = options.is_express
        };
        Publisher p(nullptr);
        ZError res =  ::z_declare_publisher(
            detail::as_owned_c_ptr(p), this->loan(), static_cast<const ::z_loaned_keyexpr_t*>(key_expr), &opts
        );
        __ZENOH_ERROR_CHECK(res, err, "Failed to declare Publisher");
        return p;
    }

    /// @brief Fetches the Zenoh IDs of all connected routers.
    std::vector<Id> get_routers_z_id(ZError* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const z_id_t& z_id) {
            out.push_back(Id(z_id));
        };
        ::z_owned_closure_zid_t c_closure;
        auto closure = new Closure<decltype(f), void, const Id&>(std::forward<decltype(f)>(f));
        ::z_closure(&c_closure, &detail::ZIdClosure<decltype(f)>::call,  &detail::ZIdClosure<decltype(f)>::drop, reinterpret_cast<void*>(closure));
        __ZENOH_ERROR_CHECK(
            ::z_info_routers_zid(this->loan(), &c_closure),
            err,
            "Failed to fetch router Ids"
        );
        return out;
    }

    /// @brief Fetches the Zenoh IDs of all connected routers.
    std::vector<Id> get_peers_z_id(ZError* err = nullptr) const {
        std::vector<Id> out;
        auto f = [&out](const z_id_t& z_id) {
            out.push_back(Id(z_id));
        };
        ::z_owned_closure_zid_t c_closure;
        auto closure = new Closure<decltype(f), void, const Id&>(std::forward<decltype(f)>(f));
        ::z_closure(&c_closure, &detail::ZIdClosure<decltype(f)>::call,  &detail::ZIdClosure<decltype(f)>::drop, reinterpret_cast<void*>(closure));
        __ZENOH_ERROR_CHECK(
            ::z_info_peers_zid(this->loan(), &c_closure),
            err,
            "Failed to fetch router Ids"
        );
        return out;
    }

#ifdef ZENOHCXX_ZENOHPICO

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task();

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task(ZError* error = nullptr);

    /// @brief Stop the read task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task();

    /// @brief Stop the read task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task(ZError* error = nullptr);

    /// @brief Start a separate task to handle the session lease.  This task will send KeepAlive messages when needed
    /// and will close the session when the lease is expired. When operating over a multicast transport, it also
    /// periodically sends the Join messages.
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_lease_task();

    /// @brief Start a separate task to handle the session lease. This task will send KeepAlive messages when needed
    /// and will close the session when the lease is expired. When operating over a multicast transport, it also
    /// periodically sends the Join messages.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_lease_task(ZError* error = nullptr);

    /// @brief Stop the lease task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task();

    /// @brief Stop the lease task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task(ZError* error = nullptr);

    /// @brief Triggers a single execution of reading procedure from the network and processes of any received the
    /// message
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool read();

    /// @brief Triggers a single execution of reading procedure from the network and processes of any received the
    /// message
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool read(ZError* error = nullptr);

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive();

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive(ZError* error = nullptr);

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join();

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join(ZError* error = nullptr);
#endif
};

struct ScoutOptions {
    /// The maximum duration in ms the scouting can take.
    size_t timeout_ms = 1000;
    /// Type of entities to scout for.
    WhatAmI what = WhatAmI::Z_WHATAMI_ROUTER_PEER;
};

/// @brief Scout for zenoh entities in the network
/// @param config ``ScoutingConfig`` to use
/// @param callback The callback to process received ``Hello``messages
/// @return 0 in case of success, negative error code otherwise
template<class F>
ZError scout(Config&& config, F&& callback, ScoutOptions&& options = {}) {
    static_assert(
        std::is_invocable_r<F, void, const Sample&>::value,
        "Callback should be callable with the following signature void callback(const zenoh::Hello& hello)"
    );
    ::z_owned_closure_query_t c_closure;
    auto closure = new Closure<F, void, const Hello&>(std::forward<F>(callback));
    ::z_closure(&c_closure, &detail::HelloClosure<F>::call, &detail::HelloClosure<F>::drop, reinterpret_cast<void*>(closure));
    ::z_scout_options_t opts = {
        .zc_timeout_ms = options.timeout,
        .zc_what = options.what
    };

    return ::z_scout(detail::as_owned_c_ptr(config), this->loan(), ::z_move(c_closure), &opts);
}


/// @brief Create a ``Session`` with the given ``Config``
/// @param config ``Config`` to use
/// @param start_background_tasks for zenoh-pico only. If true, start background threads which handles the network
/// traffic. If false, the threads should be called manually with ``Session::start_read_task`` and
/// ``Session::start_lease_task`` or methods ``Session::read``, ``Session::send_keep_alive`` and
/// ``Session::send_join`` should be called in loop.
/// @return a ``Session`` if the session was successfully created, an ``zenoh::ErrorMessage`` otherwise
Session open(Config&& config, ZError* err = nullptr) {
    return Session(std::move(config), err);
}

}
