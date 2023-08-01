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
#if defined(__ZENOHCXX_ZENOHPICO) && defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: both __ZENOHCXX_ZENOHC and __ZENOHCXX_ZENOHPICO are defined")
#endif
#if !defined(__ZENOHCXX_ZENOHPICO) && !defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: either __ZENOHCXX_ZENOHC or __ZENOHCXX_ZENOHPICO should be defined")
#endif

#ifdef __DOXYGEN__
// This is for generating documentation only. Doxygen is unable to handle namepace defined outside of the file scope.
// For real code namespaces are defined in the zenohc.hxx and zenohpico.hxx files.
namespace zenoh {
using namespace zenohcxx;
namespace z = zenohc;
#endif

class Session;
class Value;

/// Text error message returned in ``std::variant<T, ErrorMessage>`` retrun types.
typedef z::Value ErrorMessage;

/// Numeric error code value
typedef int8_t ErrNo;

/// Sample kind values.
///
/// Values:
///
///  - **Z_SAMPLE_KIND_PUT**: The Sample was issued by a "put" operation.
///  - **Z_SAMPLE_KIND_DELETE**: The Sample was issued by a "delete" operation.
typedef ::z_sample_kind_t SampleKind;

//  Zenoh encoding values.
//  These values are based on already existing HTTP MIME types and extended with other relevant encodings.

///   Values:
///     - **Z_ENCODING_PREFIX_EMPTY**: Encoding not defined.
///     - **Z_ENCODING_PREFIX_APP_OCTET_STREAM**: ``application/octet-stream``. Default value for all other cases. An
///     unknown file type should use this type.
///     - **Z_ENCODING_PREFIX_APP_CUSTOM**: Custom application type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_TEXT_PLAIN**: ``text/plain``. Default value for textual files. A textual file should be
///         human-readable and must not contain binary data.
///     - **Z_ENCODING_PREFIX_APP_PROPERTIES**: Application properties
///         type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_JSON**: ``application/json``. JSON format.
///     - **Z_ENCODING_PREFIX_APP_SQL**: Application sql type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_INTEGER**: Application integer type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_FLOAT**: Application float type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_XML**: ``application/xml``. XML.
///     - **Z_ENCODING_PREFIX_APP_XHTML_XML**: ``application/xhtml+xml``. XHTML.
///     - **Z_ENCODING_PREFIX_APP_X_WWW_FORM_URLENCODED**: ``application/x-www-form-urlencoded``. The keys and values
///     are encoded in key-value tuples separated by '&', with a '=' between the key and the value.
///     - **Z_ENCODING_PREFIX_TEXT_JSON**: Text JSON. Non IANA standard.
///     - **Z_ENCODING_PREFIX_TEXT_HTML**: ``text/html``.  HyperText Markup Language (HTML).
///     - **Z_ENCODING_PREFIX_TEXT_XML**: ``text/xml``. `Application/xml` is recommended as of RFC
///     7303 (section 4.1), but `text/xml` is still used sometimes.
///     - **Z_ENCODING_PREFIX_TEXT_CSS**: ``text/css``.  Cascading Style Sheets (CSS).
///     - **Z_ENCODING_PREFIX_TEXT_CSV**: ``text/csv``. Comma-separated values (CSV).
///     - **Z_ENCODING_PREFIX_TEXT_JAVASCRIPT**: ``text/javascript``. JavaScript.
///     - **Z_ENCODING_PREFIX_IMAGE_JPEG**: ``image/jpeg``. JPEG images.
///     - **Z_ENCODING_PREFIX_IMAGE_PNG**: ``image/png``. Portable Network Graphics.
///     - **Z_ENCODING_PREFIX_IMAGE_GIF**: ``image/gif``. Graphics Interchange Format (GIF).
typedef ::z_encoding_prefix_t EncodingPrefix;

///   Consolidation mode values.
///
///   Values:
///       - **Z_CONSOLIDATION_MODE_AUTO**: Let Zenoh decide the best consolidation mode depending on the query selector.
///       - **Z_CONSOLIDATION_MODE_NONE**: No consolidation is applied. Replies may come in any order and any number.
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
///    - **Z_CONGESTION_CONTROL_DROP**: Defines congestion control as "drop". Messages are dropped in case of congestion
///    control
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
inline z::QueryTarget query_target_default();

#ifdef __ZENOHCXX_ZENOHPICO
/// Whatami values, defined as a bitmask
///
/// \note *zenoh-pico* only
///
///
/// Values:
/// - **Z_WHATAMI_ROUTER**: Bitmask to filter Zenoh routers.
/// - **Z_WHATAMI_PEER**: Bitmask to filter for Zenoh peers.
/// - **Z_WHATAMI_CLIENT**: Bitmask to filter for Zenoh clients.
///
/// See also ``zenoh::as_cstr``
///
typedef ::z_whatami_t WhatAmI;
#endif
#ifdef __ZENOHCXX_ZENOHC
/// \note *zenoh-c* only
///
/// Whatami values, defined as a bitmask
///
/// See also ``zenoh::as_cstr``
///
enum WhatAmI {
    /// Bitmask to filter Zenoh routers
    Z_WHATAMI_ROUTER = 1,
    /// Bitmask to filter for Zenoh peers
    Z_WHATAMI_PEER = 1 << 1,
    /// Bitmask to filter for Zenoh clients
    Z_WHATAMI_CLIENT = 1 << 2
};
#endif

/// \anchor as_cstr
/// Returns a string representation of the given ``WhatAmI`` value.
/// @param whatami the ``WhatAmI`` value
/// @return a string representation of the given ``WhatAmI`` value
inline const char* as_cstr(z::WhatAmI whatami);

#ifdef __ZENOHCXX_ZENOHC
/// Initializes logger (zenoh-c library only)
/// User may set environment variable RUST_LOG to values debug|info|warn|error to show diagnostic output
void init_logger();
#endif

template <typename Z_STR_ARRAY_T>
struct _StrArrayView : Copyable<Z_STR_ARRAY_T> {
    typedef decltype(Z_STR_ARRAY_T::val) VALTYPE;
    using Copyable<Z_STR_ARRAY_T>::Copyable;

    /// @name Constructors

    /// Constructs an uninitialized instance
    _StrArrayView() : Copyable<Z_STR_ARRAY_T>({0, nullptr}) {}
    /// Constructs an instance from a ``std::vector`` of ``const char*``
    /// @param v the ``std::vector`` of ``const char*``
    _StrArrayView(const std::vector<const char*>& v)
        : Copyable<Z_STR_ARRAY_T>({v.size(), const_cast<VALTYPE>(&v[0])}) {}
    /// Constructs an instance from an array of ``const char*``
    /// @param v the array of ``const char*``
    /// @param len the length of the array
    ////
    _StrArrayView(const char** v, size_t len) : Copyable<Z_STR_ARRAY_T>({len, const_cast<VALTYPE>(v)}) {}
    /// Constructs an instance from an constant array of ``const char*``
    /// @param v the array of ``const char*``
    /// @param len the length of the array
    _StrArrayView(const char* const* v, size_t len)
        : Copyable<Z_STR_ARRAY_T>({len, const_cast<VALTYPE>(v)}) {}

    /// @name Operators

    /// Operator to access an element of the array by index
    /// @param pos the index of the element
    /// @return the element at the given index
    const char* operator[](size_t pos) const { return Copyable<Z_STR_ARRAY_T>::val[pos]; }

    /// @name Methods

    /// Returns the length of the array
    /// @return the length of the array
    size_t get_len() const { return Copyable<Z_STR_ARRAY_T>::len; }
};
/// Represents non-owned read only array of ``char*``
/// Impmemented in a template ``zenoh::_StrArrayView`` to handle with different const definitions in ``::z_str_array_t``
/// struct in zenoh-pico and zenoh-c
struct StrArrayView : z::_StrArrayView<::z_str_array_t> {
    using _StrArrayView<::z_str_array_t>::_StrArrayView;
};

/// Represents non-owned read only array of bytes
class BytesView : public Copyable<::z_bytes_t> {
   public:
    using Copyable::Copyable;
    /// @name Constructors

    /// @brief Constructs an uninitialized instance
    BytesView(nullptr_t) : Copyable(init(nullptr, 0)) {}
    /// Constructs an instance from an array of bytes
    /// @param s the array of bytes
    /// @param _len the length of the array
    BytesView(const void* s, size_t _len) : Copyable(init(reinterpret_cast<const uint8_t*>(s), _len)) {}
    /// Constructs an instance from a null-terminated string
    /// @param s the null-terminated string
    BytesView(const char* s) : Copyable({s ? strlen(s) : 0, reinterpret_cast<const uint8_t*>(s)}) {}
    /// Constructs an instance from a ``std::vector`` of type ``T``
    /// @param v the ``std::vector`` of type ``T``
    template <typename T>
    BytesView(const std::vector<T>& v)
        : Copyable({v.size() * sizeof(T), reinterpret_cast<const uint8_t*>(&v[0])}) {}
    /// Constructs an instance from a ``std::string_view``
    /// @param s the ``std::string_view``
    BytesView(const std::string_view& s)
        : Copyable({s.length(), reinterpret_cast<const uint8_t*>(s.data())}) {}
    /// Constructs an instance from a ``std::string``
    /// @param s the ``std::string``
    BytesView(const std::string& s)
        : Copyable({s.length(), reinterpret_cast<const uint8_t*>(s.data())}) {}

    /// @name Operators

    /// Returns a ``std::string_view`` representation of the array of bytes
    /// @return a ``std::string_view`` representation of the array of bytes
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start), len); }
    /// Compares two instances of ``BytesView``
    /// @param v the other instance of ``BytesView``
    /// @return true if the two instances are equal, false otherwise
    bool operator==(const BytesView& v) const { return as_string_view() == v.as_string_view(); }
    /// Compares two instances of ``BytesView``
    /// @param v the other instance of ``BytesView``
    /// @return true if the two instances are not equal, false otherwise
    bool operator!=(const BytesView& v) const { return !operator==(v); }

    /// @name Methods

    /// Returns the length of the array
    /// @return the length of the array
    size_t get_len() const { return len; }
    /// Checks if the array is initialized
    /// @return true if the array is initialized
    bool check() const { return ::z_bytes_check(this); }

   private:
    ::z_bytes_t init(const uint8_t* start, size_t len);
};

///  Represents a Zenoh ID.
///  In general, valid Zenoh IDs are LSB-first 128bit unsigned and non-zero integers.
///
/// See also: \ref operator_id_out "operator<<(std::ostream& os, const z::Id& id)"
struct Id : public Copyable<::z_id_t> {
    using Copyable::Copyable;

    /// @name Methods

    /// Checks if the ID is valid
    /// @return true if the ID is valid
    bool is_some() const { return id[0] != 0; }
};
/// \anchor operator_id_out
///
/// @brief Output operator for Id
/// @param os the output stream
/// @param id reference to the Id
/// @return the output stream
std::ostream& operator<<(std::ostream& os, const z::Id& id);

/// The non-owning read-only view to a "hello" message returned by a zenoh entity as a reply to a "scout"
/// message.
struct HelloView : public Copyable<::z_hello_t> {
    using Copyable::Copyable;

    /// @name Methods

    /// @brief Get ``Id`` of the entity
    /// @return ``Id`` of the entity
    const z::Id& get_id() const;
    /// @brief Get the ``zenoh::WhatAmI`` of the entity
    /// @return ``zenoh::WhatAmI`` of the entity
    z::WhatAmI get_whatami() const { return static_cast<z::WhatAmI>(whatami); }
    /// @brief Get the array of locators of the entity
    /// @return the array of locators of the entity
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
// Keyexpr-related string functions
//
inline bool keyexpr_canonize(std::string& s, ErrNo& error);
inline bool keyexpr_canonize(std::string& s);
inline bool keyexpr_is_canon(const std::string_view& s, ErrNo& error);
inline bool keyexpr_is_canon(const std::string_view& s);

/// The non-owning read-only view to a key expression in Zenoh.
struct KeyExprView : public Copyable<::z_keyexpr_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Constructs an uninitialized instance
    KeyExprView(nullptr_t) : Copyable(::z_keyexpr(nullptr)) {}
    /// @brief Constructs an instance from a null-terminated string representing a key expression.
    KeyExprView(const char* name) : Copyable(::z_keyexpr(name)) {}
    /// @brief Constructs an instance from a null-terminated string representing a key expression withot validating it
    /// In debug mode falis on assert if passed string is not a valid key expression
    /// @param name the null-terminated string representing a key expression
    /// @param _unchecked the empty type used to distinguish checked and unchecked construncting of KeyExprView
    KeyExprView(const char* name, z::KeyExprUnchecked _unchecked) : Copyable(::z_keyexpr_unchecked(name)) {
        assert(keyexpr_is_canon(name));
    }
    /// @brief Constructs an instance from a ``std::string`` representing a key expression.
    /// @param name the string representing a key expression
    KeyExprView(const std::string& name) : Copyable(::z_keyexpr(name.c_str())) {}
#ifdef __ZENOHCXX_ZENOHC
    /// @brief Constructs an instance from ``std::string_view`` representing a key expression.
    /// @param name the ``std::string_view`` representing a key expression
    /// @note zenoh-c only. Zenoh-pico supports only null-terminated key expression strings
    KeyExprView(const std::string_view& name) : Copyable(::zc_keyexpr_from_slice(name.data(), name.length())) {}
    /// @brief Constructs an instance from ``std::string_view`` representing a key expression without validating it
    /// @param name the ``std::string_view`` representing a key expression
    /// @note zenoh-c only. Zenoh-pico supports only null-terminated key expression strings
    KeyExprView(const std::string_view& name, z::KeyExprUnchecked)
        : Copyable(::zc_keyexpr_from_slice_unchecked(name.data(), name.length())) {
        assert(keyexpr_is_canon(name));
    }
#endif

    /// @name Operators

    // operator == between keyexprs purposedly not defided to avoid ambiguity: it's not obvious is string
    // equality or z_keyexpr_equals would be used by operator==

    /// @brief Equality operator between a key expression and a string
    /// @param v ``std::string_view`` representing a key expression
    /// @return true if the key expression and the string are equal
    bool operator==(const std::string_view& v) const { return as_string_view() == v; }

    /// @brief Inequality operator between a key expression and a string
    /// @param v ``std::string_view`` representing a key expression
    /// @return true if the key expression and the string are not equal
    bool operator!=(const std::string_view& v) const { return !operator==(v); }

    /// @name Methods

    /// @brief Checks if the key expression is valid
    /// @return true if the key expression is valid
    bool check() const { return ::z_keyexpr_is_initialized(this); }
    /// @brief Return the key ``BytesView`` on the key expression
    /// @return ``BytesView`` structure pointing to the key expression
    z::BytesView as_bytes() const { return z::BytesView{::z_keyexpr_as_bytes(*this)}; }
    /// @brief Return the key expression as a ``std::string_view``
    /// @return ``std::string_view`` representing the key expression
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }
#ifdef __ZENOHCXX_ZENOHC
    // operator += purposedly not defined to not provoke ambiguity between concat (which
    // mechanically connects strings) and join (which works with path elements)

    /// @brief Concatenate the key expression and a string
    /// @param s ``std::string_view`` representing a key expression
    /// @return Newly allocated key expression ``zenoh::KeyExpr``
    /// @note zenoh-c only
    z::KeyExpr concat(const std::string_view& s) const;

    /// @brief Join key expression with another key expression, inserting a separator between them
    /// @param v the key expression to join with
    /// @return Newly allocated key expression ``zenoh::KeyExpr``
    /// @note zenoh-c only
    z::KeyExpr join(const KeyExprView& v) const;
#endif

    /// @brief Checks if the key expression is equal to another key expression
    /// @param v Another key expression
    /// @param error Error code returned by ``::z_keyexpr_equals`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expression is equal to the other key expression
    bool equals(const KeyExprView& v, ErrNo& error) const;

    /// @brief Checks if the key expression is equal to another key expression
    /// @param v Another key expression
    /// @return true the key expression is equal to the other key expression
    bool equals(const KeyExprView& v) const;

    /// @brief Checks if the key expression includes another key expression, i.e. if the set defined by the key
    /// expression contains the set defined by the other key expression
    /// @param v Another key expression
    /// @param error Error code returned by ``::z_keyexpr_includes`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expression includes the other key expression
    bool includes(const KeyExprView& v, ErrNo& error) const;

    /// @brief Checks if the key expression includes another key expression, i.e. if the set defined by the key
    /// expression contains the set defined by the other key expression
    /// @param v Another key expression
    /// @return true the key expression includes the other key expression
    bool includes(const KeyExprView& v) const;

    /// @brief Checks if the key expression intersects with another key expression, i.e. there exists at least one key
    /// which is contained in both of the sets defined by the key expressions
    /// @param v Another key expression
    /// @param error Error code returned by ``::z_keyexpr_intersects`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expression intersects with the other key expression
    bool intersects(const KeyExprView& v, ErrNo& error) const;

    /// @brief Checks if the key expression intersects with another key expression, i.e. there exists at least one key
    /// which is contained in both of the sets defined by the key expressions
    /// @param v Another key expression
    /// @return true the key expression intersects with the other key expression
    bool intersects(const KeyExprView& v) const;
};

/// The encoding of a payload, in a MIME-like format.
///
/// For wire and matching efficiency, common MIME types are represented using an integer as "prefix", and a "suffix"
/// may be used to either provide more detail, or in combination with the **Z_ENCODING_PREFIX_EMPTY** value of
/// ``zenoh::EncodingPrefix`` to write arbitrary MIME types.
///
struct Encoding : public Copyable<::z_encoding_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Default encoding
    Encoding() : Copyable(::z_encoding_default()) {}

    /// @brief Encoding with a prefix
    Encoding(EncodingPrefix _prefix) : Copyable(::z_encoding(_prefix, nullptr)) {}

    /// @brief Encoding with a prefix and a suffix
    Encoding(EncodingPrefix _prefix, const char* _suffix) : Copyable(::z_encoding(_prefix, _suffix)) {}

    /// @name Methods

    /// @brief Set the prefix for the encoding
    /// @param _prefix value of ``zenoh::EncodingPrefix`` type
    /// @return Reference to the ``Encoding`` object
    Encoding& set_prefix(EncodingPrefix _prefix) {
        prefix = _prefix;
        return *this;
    }

    /// @brief Set the suffix for the encoding
    /// @param _suffix ``zenoh::BytesView`` representing the suffix
    /// @return Reference to the ``Encoding`` object
    Encoding& set_suffix(const z::BytesView& _suffix) {
        suffix = _suffix;
        return *this;
    }

    /// @brief Get the prefix of the encoding
    /// @return value of ``zenoh::EncodingPrefix`` type
    EncodingPrefix get_prefix() const { return prefix; }

    /// @brief Get the suffix of the encoding
    /// @return ``zenoh::BytesView`` representing the suffix
    const z::BytesView& get_suffix() const { return static_cast<const z::BytesView&>(suffix); }

    /// @name Operators

    /// @brief Equality operator
    /// @param v other ``Encoding`` object
    /// @return true if the encodings are equal
    bool operator==(const Encoding& v) const {
        return get_prefix() == v.get_prefix() && get_suffix() == v.get_suffix();
    }

    /// @brief Inequality operator
    /// @param v other ``Encoding`` object
    /// @return true if the encodings are not equal
    bool operator!=(const Encoding& v) const { return !operator==(v); }
};

/// Represents timestamp value in zenoh
struct Timestamp : Copyable<::z_timestamp_t> {
    using Copyable::Copyable;
    // TODO: add utility methods to interpret time as mils, seconds, minutes, etc

    /// @name Methods

    /// @brief Get the time part of timestamp in <a
    /// href=https://docs.rs/zenoh/0.7.2-rc/zenoh/time/struct.NTP64.html>NTP64</a> format
    /// @return timestamp value
    uint64_t get_time() const { return time; }

    /// @brief Get the unique part of the timestamp
    /// @return unique id
    z::Id get_id() const { return id; }

    /// @brief Check if the timestamp is valid
    /// @return true if the timestamp is valid
    bool check() const { return ::z_timestamp_check(*this); }
};

#ifdef __ZENOHCXX_ZENOHC
//
// Owned reference-counted payload object
// Availabel only in zenoh-c where underlying buffer is reference-counted and it's possible to
// take this buffer for further processing. It can be convenient if it's necessary to resend the
// buffer to one or multiple receivers without copying it.
//
class Payload : public Owned<::zc_owned_payload_t> {
   public:
    using Owned::Owned;
    Payload rcinc() const { return Payload(::zc_payload_rcinc(&_0)); }
    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(_0.payload); }
};

//
// Memory buffer returned by shared memory manager
//
class Shmbuf : public Owned<::zc_owned_shmbuf_t> {
   public:
    using Owned::Owned;
    uintptr_t get_capacity() const { return ::zc_shmbuf_capacity(&_0); }
    uintptr_t get_length() const { return ::zc_shmbuf_length(&_0); }
    void set_length(uintptr_t length) { ::zc_shmbuf_set_length(&_0, length); }
    z::Payload into_payload() { return z::Payload(std::move(::zc_shmbuf_into_payload(&_0))); }
    uint8_t* ptr() const { return ::zc_shmbuf_ptr(&_0); }
    char* char_ptr() const { return reinterpret_cast<char*>(ptr()); }
    std::string_view as_string_view() const {
        return std::string_view(reinterpret_cast<const char*>(ptr()), get_length());
    }
};

//
// Shared memory manager
//

class ShmManager;
std::variant<z::ShmManager, z::ErrorMessage> shm_manager_new(const z::Session& session, const char* id, uintptr_t size);

class ShmManager : public Owned<::zc_owned_shm_manager_t> {
   public:
    using Owned::Owned;

    friend std::variant<z::ShmManager, z::ErrorMessage> z::shm_manager_new(const z::Session& session, const char* id,
                                                                           uintptr_t size);

    std::variant<z::Shmbuf, z::ErrorMessage> alloc(uintptr_t capacity) const;
    uintptr_t defrag() const { return ::zc_shm_defrag(&_0); }
    uintptr_t gc() const { return ::zc_shm_gc(&_0); }

   private:
    ShmManager(const z::Session& session, const char* id, uintptr_t size);
};

#endif

//
// A data sample.
//
// A sample is the value associated to a given resource at a given point in time.
//
struct Sample : public Copyable<::z_sample_t> {
    using Copyable::Copyable;
    const z::KeyExprView& get_keyexpr() const { return static_cast<const z::KeyExprView&>(keyexpr); }
    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(payload); }
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }
    SampleKind get_kind() const { return kind; }
#ifdef __ZENOHCXX_ZENOHC
    z::Payload sample_payload_rcinc() const {
        auto p = ::zc_sample_payload_rcinc(static_cast<const ::z_sample_t*>(this));
        return z::Payload(std::move(p));
    }
#endif
};

//
// A zenoh value
//
struct Value : public Copyable<::z_value_t> {
    using Copyable::Copyable;
    Value(const z::BytesView& payload, const z::Encoding& encoding)
        : Copyable({payload, encoding}) {}
    Value(const z::BytesView& payload) : Value(payload, z::Encoding()) {}
    Value(const char* payload) : Value(payload, z::Encoding()) {}

    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(payload); }
    Value& set_payload(const z::BytesView& _payload) {
        payload = _payload;
        return *this;
    }

    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }
    Value& set_encoding(const z::Encoding& _encoding) {
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
    GetOptions& set_consolidation(z::QueryConsolidation v) {
        consolidation = v;
        return *this;
    }
    GetOptions& set_value(z::Value v) {
        value = v;
        return *this;
    }
    QueryTarget get_target() const { return target; }
    const z::QueryConsolidation& get_consolidation() const {
        return static_cast<const z::QueryConsolidation&>(consolidation);
    }
    const z::Value& get_value() const { return static_cast<const z::Value&>(value); }
    bool operator==(const GetOptions& v) const {
        return get_target() == v.get_target() && get_consolidation() == v.get_consolidation() &&
               get_value() == v.get_value();
    }
    bool operator!=(const GetOptions& v) const { return !operator==(v); }
};

//
// Options passed to the put operation
//
struct PutOptions : public Copyable<::z_put_options_t> {
    using Copyable::Copyable;
    PutOptions() : Copyable(::z_put_options_default()) {}
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }
    PutOptions& set_encoding(z::Encoding e) {
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
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }
    QueryReplyOptions& set_encoding(z::Encoding e) {
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
    z::KeyExprView get_keyexpr() const { return z::KeyExprView(::z_query_keyexpr(this)); }
    z::BytesView get_parameters() const { return z::BytesView(::z_query_parameters(this)); }
    z::Value get_value() const { return z::Value(::z_query_value(this)); }

    bool reply(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions& options,
               ErrNo& error) const;
    bool reply(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions& options) const;
    bool reply(z::KeyExprView key, const z::BytesView& payload, ErrNo& error) const;
    bool reply(z::KeyExprView key, const z::BytesView& payload) const;

   private:
    bool reply_impl(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions* options,
                    ErrNo& error) const;
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
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }
    PublisherPutOptions& set_encoding(z::Encoding e) {
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
    bool operator==(const z::PublisherOptions& v) const { return true; }
    bool operator!=(const z::PublisherOptions& v) const { return !operator==(v); }
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
    z::KeyExprView as_keyexpr_view() const { return z::KeyExprView(::z_keyexpr_loan(&_0)); }
    operator z::KeyExprView() const { return as_keyexpr_view(); }
    z::BytesView as_bytes() const { return as_keyexpr_view().as_bytes(); }
    std::string_view as_string_view() const { return as_keyexpr_view().as_string_view(); }
    bool operator==(const std::string_view& v) { return as_string_view() == v; }
#ifdef __ZENOHCXX_ZENOHC
    z::KeyExpr concat(const std::string_view& s) const { return as_keyexpr_view().concat(s); }
    z::KeyExpr join(const z::KeyExprView& v) const { return as_keyexpr_view().join(v); }
#endif
    bool equals(const z::KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().equals(v, error); }
    bool equals(const z::KeyExprView& v) const { return as_keyexpr_view().equals(v); }
    bool includes(const z::KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().includes(v, error); }
    bool includes(const z::KeyExprView& v) const { return as_keyexpr_view().includes(v); }
    bool intersects(const z::KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().intersects(v, error); }
    bool intersects(const z::KeyExprView& v) const { return as_keyexpr_view().intersects(v); }
};

class ScoutingConfig;

//
// Zenoh config
//
class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
#ifdef __ZENOHCXX_ZENOHC
    z::Str get(const char* key) const { return z::Str(::zc_config_get(::z_config_loan(&_0), key)); }
    z::Str to_string() const { return z::Str(::zc_config_to_string(::z_config_loan(&_0))); }
    bool insert_json(const char* key, const char* value) {
        return ::zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    const char* get(uint8_t key) const { return ::zp_config_get(::z_config_loan(&_0), key); }
    bool insert(uint8_t key, const char* value);
    bool insert(uint8_t key, const char* value, ErrNo& error);
#endif
    z::ScoutingConfig create_scouting_config();
};

#ifdef __ZENOHCXX_ZENOHC
inline z::Config config_peer() { return z::Config(::z_config_peer()); }
std::variant<z::Config, ErrorMessage> config_from_file(const char* path);
std::variant<z::Config, ErrorMessage> config_from_str(const char* s);
std::variant<z::Config, ErrorMessage> config_client(const z::StrArrayView& peers);
std::variant<z::Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers);
#endif

//
// An owned reply to get operation
//
class Reply : public Owned<::z_owned_reply_t> {
   public:
    using Owned::Owned;
    bool is_ok() const { return ::z_reply_is_ok(&_0); }
    std::variant<z::Sample, ErrorMessage> get() const {
        if (is_ok()) {
            return z::Sample{::z_reply_ok(&_0)};
        } else {
            return z::ErrorMessage{::z_reply_err(&_0)};
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
    bool put(const z::BytesView& payload, const z::PublisherPutOptions& options, ErrNo& error);
    bool put(const z::BytesView& payload, ErrNo& error);
    bool put(const z::BytesView& payload, const z::PublisherPutOptions& options);
    bool put(const z::BytesView& payload);
    bool delete_resource(const z::PublisherDeleteOptions& options, ErrNo& error);
    bool delete_resource(ErrNo& error);
    bool delete_resource(const z::PublisherDeleteOptions& options);
    bool delete_resource();

#ifdef __ZENOHCXX_ZENOHC
    bool put_owned(z::Payload&& payload, const z::PublisherPutOptions& options, ErrNo& error);
    bool put_owned(z::Payload&& payload, ErrNo& error);
    bool put_owned(z::Payload&& payload, const z::PublisherPutOptions& options);
    bool put_owned(z::Payload&& payload);
#endif

   private:
    bool put_impl(const z::BytesView& payload, const z::PublisherPutOptions* options, ErrNo& error);
    bool delete_impl(const z::PublisherDeleteOptions* options, ErrNo& error);
#ifdef __ZENOHCXX_ZENOHC
    bool put_owned_impl(z::Payload&& payload, const z::PublisherPutOptions* options, ErrNo& error);
#endif
};

//
// A zenoh-allocated hello message returned by a zenoh entity to a scout message sent with scout operation
//
class Hello : public Owned<::z_owned_hello_t> {
   public:
    using Owned::Owned;
    operator z::HelloView() const { return z::HelloView(::z_hello_loan(&_0)); }
};

//
//  Represents the reply closure.
//
typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, z::Reply> ClosureReply;

//
//  Represents the query closure.
//
typedef ClosureConstRefParam<::z_owned_closure_query_t, ::z_query_t, z::Query> ClosureQuery;

//
//  Represents the sample closure.
//
typedef ClosureConstRefParam<::z_owned_closure_sample_t, ::z_sample_t, z::Sample> ClosureSample;

//
//  Represents the zenoh ID closure.
//
typedef ClosureConstRefParam<::z_owned_closure_zid_t, ::z_id_t, z::Id> ClosureZid;

//
// Represents the scouting closure
//
typedef ClosureMoveParam<::z_owned_closure_hello_t, ::z_owned_hello_t, z::Hello> ClosureHello;

//
// Zenoh scouting config and function
//
class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
    ScoutingConfig(z::Config& config) : Owned(std::move(ScoutingConfig(config))) {}
};

bool scout(z::ScoutingConfig&& config, z::ClosureHello&& callback, ErrNo& error);
bool scout(z::ScoutingConfig&& config, z::ClosureHello&& callback);

//
// Zenoh session
//

class Session;
std::variant<z::Session, z::ErrorMessage> open(z::Config&& config, bool start_background_tasks = true);

class Session : public Owned<::z_owned_session_t> {
   public:
    using Owned::Owned;

    Session(Session&& other) : Owned(std::move(other)) {}
    Session(nullptr_t) : Owned(nullptr) {}
    Session&& operator=(Session&& other);
    void drop();
    ~Session() { drop(); }

#ifdef __ZENOHCXX_ZENOHC
    Session rcinc() { return Session(::zc_session_rcinc(::z_session_loan(&_0))); }
#endif

    z::Id info_zid() const { return ::z_info_zid(::z_session_loan(&_0)); }

    friend std::variant<z::Session, z::ErrorMessage> z::open(z::Config&& config, bool start_background_tasks);

    z::KeyExpr declare_keyexpr(const z::KeyExprView& keyexpr);
    bool undeclare_keyexpr(z::KeyExpr&& keyexpr, ErrNo& error);
    bool undeclare_keyexpr(z::KeyExpr&& keyexpr);
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, const z::GetOptions& options,
             ErrNo& error);
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, const z::GetOptions& options);
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, ErrNo& error);
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback);
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, const z::PutOptions& options, ErrNo& error);
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, const z::PutOptions& options);
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, ErrNo& error);
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload);
    bool delete_resource(z::KeyExprView keyexpr, const z::DeleteOptions& options, ErrNo& error);
    bool delete_resource(z::KeyExprView keyexpr, const z::DeleteOptions& options);
    bool delete_resource(z::KeyExprView keyexpr, ErrNo& error);
    bool delete_resource(z::KeyExprView keyexpr);
#ifdef __ZENOHCXX_ZENOHC
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const z::PutOptions& options, ErrNo& error);
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const z::PutOptions& options);
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, ErrNo& error);
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload);
#endif
    std::variant<z::Queryable, ErrorMessage> declare_queryable(z::KeyExprView keyexpr, z::ClosureQuery&& callback,
                                                               const z::QueryableOptions& options);
    std::variant<z::Queryable, ErrorMessage> declare_queryable(z::KeyExprView keyexpr, z::ClosureQuery&& callback);
    std::variant<z::Subscriber, ErrorMessage> declare_subscriber(z::KeyExprView keyexpr, z::ClosureSample&& callback,
                                                                 const z::SubscriberOptions& options);
    std::variant<z::Subscriber, ErrorMessage> declare_subscriber(z::KeyExprView keyexpr, z::ClosureSample&& callback);
    std::variant<z::PullSubscriber, ErrorMessage> declare_pull_subscriber(z::KeyExprView keyexpr,
                                                                          z::ClosureSample&& callback,
                                                                          const z::PullSubscriberOptions& options);
    std::variant<z::PullSubscriber, ErrorMessage> declare_pull_subscriber(z::KeyExprView keyexpr,
                                                                          z::ClosureSample&& callback);
    std::variant<z::Publisher, ErrorMessage> declare_publisher(z::KeyExprView keyexpr,
                                                               const z::PublisherOptions& options);
    std::variant<z::Publisher, ErrorMessage> declare_publisher(z::KeyExprView keyexpr);
    bool info_routers_zid(z::ClosureZid&& callback, ErrNo& error);
    bool info_routers_zid(z::ClosureZid&& callback);
    bool info_peers_zid(z::ClosureZid&& callback, ErrNo& error);
    bool info_peers_zid(z::ClosureZid&& callback);

#ifdef __ZENOHCXX_ZENOHPICO
    bool start_read_task();
    bool start_read_task(ErrNo& error);
    bool stop_read_task();
    bool stop_read_task(ErrNo& error);

    bool start_lease_task();
    bool start_lease_task(ErrNo& error);
    bool stop_lease_task();
    bool stop_lease_task(ErrNo& error);

    bool read();
    bool read(ErrNo& error);

    bool send_keep_alive();
    bool send_keep_alive(ErrNo& error);

    bool send_join();
    bool send_join(ErrNo& error);
#endif

   private:
    Session(z::Config&& v) : Owned(_z_open(std::move(v))) {}
    bool undeclare_keyexpr_impl(z::KeyExpr&& keyexpr, ErrNo& error);
    bool get_impl(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback,
                  const z::GetOptions* options, ErrNo& error);
    bool put_impl(z::KeyExprView keyexpr, const z::BytesView& payload, const z::PutOptions* options, ErrNo& error);
    bool delete_impl(z::KeyExprView keyexpr, const z::DeleteOptions* options, ErrNo& error);
#ifdef __ZENOHCXX_ZENOHC
    bool put_owned_impl(z::KeyExprView keyexpr, z::Payload&& payload, const z::PutOptions* options, ErrNo& error);
#endif
    std::variant<z::Queryable, ErrorMessage> declare_queryable_impl(z::KeyExprView keyexpr, z::ClosureQuery&& callback,
                                                                    const z::QueryableOptions* options);
    std::variant<z::Subscriber, ErrorMessage> declare_subscriber_impl(z::KeyExprView keyexpr,
                                                                      z::ClosureSample&& callback,
                                                                      const z::SubscriberOptions* options);
    std::variant<z::PullSubscriber, ErrorMessage> declare_pull_subscriber_impl(z::KeyExprView keyexpr,
                                                                               z::ClosureSample&& callback,
                                                                               const z::PullSubscriberOptions* options);
    std::variant<z::Publisher, ErrorMessage> declare_publisher_impl(z::KeyExprView keyexpr,
                                                                    const z::PublisherOptions* options);

    static ::z_owned_session_t _z_open(z::Config&& v);
};

#ifdef __ZENOHCXX_ZENOHC

class ClosureReplyChannelSend : public ClosureReply {
   public:
    using ClosureReply::ClosureReply;
};

class ClosureReplyChannelRecv
    : public ClosureMoveParam<::z_owned_reply_channel_closure_t, ::z_owned_reply_t, z::Reply> {
   public:
    using ClosureMoveParam::ClosureMoveParam;
};

//
// Creates a new blocking fifo channel, returned as a pair of closures.
//
// The `send` end should be passed as callback to a `z_get` call.
//
// The `recv` end is a synchronous closure to be called from user code. It will block until either a Reply is available,
// which it will then return; or until the `send` closure is dropped and all replies have been consumed,
// at which point it will return an invalidated Reply and so will further calls.
//

inline std::pair<z::ClosureReplyChannelSend, z::ClosureReplyChannelRecv> reply_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

//
// Creates a new non-blocking fifo channel, returned as a pair of closures.
//
// The `send` end should be passed as callback to a `z_get` call.
//
// The `recv` end is a synchronous closure to be called from user code. It will block until either a Reply is available,
// which it will then return; or until the `send` closure is dropped and all replies have been consumed,
// at which point it will return an invalidated Reply and so will further calls.
//
inline std::pair<z::ClosureReplyChannelSend, z::ClosureReplyChannelRecv> reply_non_blocking_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_non_blocking_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

#endif

#ifdef __DOXYGEN__
}  // end of namespace zenoh
#endif
