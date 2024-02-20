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
#endif

class Session;
struct Value;

/// Text error message returned in ``std::variant<T, ErrorMessage>`` return types.
/// The message is a sting represented as ``zenoh::Value`` object. See also ``zenoh::expect`` function
/// which eases access to the return value in this variant type.
typedef z::Value ErrorMessage;

/// Uility function which either returns the value or throws an exception with the ``zenoh::ErrorMessage`` value
/// @param v the ``std::variant<T, zenoh:::ErrorMessage>`` value
/// @return the value of type ``T`` if ``std::variant<T, zenoh::ErrorMessage>`` value contains it
/// @throws  ``zenoh::ErrorMessage`` if ``std::variant<T, zenoh::ErrorMessage>`` value contains it
template <typename T>
inline T expect(std::variant<T, z::ErrorMessage>&& v) {
    if (v.index() == 1) {
        throw std::get<z::ErrorMessage>(std::move(v));
    } else {
        return std::get<T>(std::move(v));
    }
}

/// Numeric error code value. This is a value < -1 returned by zenoh-c functions
typedef int8_t ErrNo;

/// ``zenoh::Sample`` kind values.
///
/// Values:
///
///  - **Z_SAMPLE_KIND_PUT**: The Sample was issued by a "put" operation.
///  - **Z_SAMPLE_KIND_DELETE**: The Sample was issued by a "delete" operation.
typedef ::z_sample_kind_t SampleKind;

///  Zenoh encoding values for ``zenoh::Encoding``
///  These values are based on already existing HTTP MIME types and extended with other relevant encodings.
///
///   Values:
///     - **Z_ENCODING_PREFIX_EMPTY**: Encoding not defined.
///     - **Z_ENCODING_PREFIX_APP_OCTET_STREAM**: ``application/octet-stream``. Default value for all other cases.
///     An unknown file type should use this type.
///     - **Z_ENCODING_PREFIX_APP_CUSTOM**: Custom application type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_TEXT_PLAIN**: ``text/plain``. Default value for textual files. A textual file should
///     be
///         human-readable and must not contain binary data.
///     - **Z_ENCODING_PREFIX_APP_PROPERTIES**: Application properties
///         type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_JSON**: ``application/json``. JSON format.
///     - **Z_ENCODING_PREFIX_APP_SQL**: Application sql type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_INTEGER**: Application integer type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_FLOAT**: Application float type. Non IANA standard.
///     - **Z_ENCODING_PREFIX_APP_XML**: ``application/xml``. XML.
///     - **Z_ENCODING_PREFIX_APP_XHTML_XML**: ``application/xhtml+xml``. XHTML.
///     - **Z_ENCODING_PREFIX_APP_X_WWW_FORM_URLENCODED**: ``application/x-www-form-urlencoded``. The keys and
///     values are encoded in key-value tuples separated by '&', with a '=' between the key and the value.
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

 /// QoS settings of zenoh message.
 ///
struct QoS : public Copyable<::z_qos_t> {
    using Copyable::Copyable;
    /// @brief Returns message priority.
    Priority get_priority() const { return ::z_qos_get_priority(*this); }
    /// @brief Returns message congestion control.
    CongestionControl get_congestion_control() const { return ::z_qos_get_congestion_control(*this); }
    /// @brief Returns message express flag. If set to true, the message is not batched to reduce the latency.
    bool get_express() const { return ::z_qos_get_express(*this); }
    /// @brief Crates default QoS settings.
    QoS() : Copyable(z_qos_default()) {};
};

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

#ifdef __DOXYGEN__
}
namespace zenohpico {
#endif

/// Whatami values, defined as a bitmask
///
/// Values:
/// - **Z_WHATAMI_ROUTER**: Bitmask to filter Zenoh routers.
/// - **Z_WHATAMI_PEER**: Bitmask to filter for Zenoh peers.
/// - **Z_WHATAMI_CLIENT**: Bitmask to filter for Zenoh clients.
///
/// See also ``zenoh::as_cstr``
///
/// @note zenoh-pico implementation
typedef ::z_whatami_t WhatAmI;

#ifdef __DOXYGEN__
}
namespace zenoh {
#endif

#endif

#ifdef __ZENOHCXX_ZENOHC

#ifdef __DOXYGEN__
}
namespace zenohc {
#endif

/// @brief Whatami values, defined as a bitmask
///
/// See also ``zenoh::as_cstr``
/// @note zenoh-c implementation
enum WhatAmI {
    /// Bitmask to filter Zenoh routers
    Z_WHATAMI_ROUTER = 1,
    /// Bitmask to filter for Zenoh peers
    Z_WHATAMI_PEER = 1 << 1,
    /// Bitmask to filter for Zenoh clients
    Z_WHATAMI_CLIENT = 1 << 2
};

#ifdef __DOXYGEN__
}
namespace zenoh {
#endif

#endif

/// @brief Returns a string representation of the given ``zenohc::WhatAmI``
/// (or the ``zenohpico::WhatAmI``) value.
/// @param whatami the ``zenohc::WhatAmI`` / ``zenohpico::WhatAmI`` value
/// @return a string representation of the given value
inline const char* as_cstr(z::WhatAmI whatami);

#ifdef __ZENOHCXX_ZENOHC
/// @brief Initializes logger
///
/// User may set environment variable RUST_LOG to values *debug* | *info* | *warn* | *error* to show diagnostic output
///
/// @note zenoh-c only
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
    _StrArrayView(const char* const* v, size_t len) : Copyable<Z_STR_ARRAY_T>({len, const_cast<VALTYPE>(v)}) {}

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
    BytesView(const char* s) : Copyable(init(reinterpret_cast<const uint8_t*>(s), s ? strlen(s) : 0)) {}
    /// Constructs an instance from a ``std::vector`` of type ``T``
    /// @param v the ``std::vector`` of type ``T``
    template <typename T>
    BytesView(const std::vector<T>& v)
        : Copyable(init(reinterpret_cast<const uint8_t*>(&v[0]), v.size() * sizeof(T))) {}
    /// Constructs an instance from a ``std::string_view``
    /// @param s the ``std::string_view``
    BytesView(const std::string_view& s) : Copyable(init(reinterpret_cast<const uint8_t*>(s.data()), s.length())) {}
    /// Constructs an instance from a ``std::string``
    /// @param s the ``std::string``
    BytesView(const std::string& s) : Copyable(init(reinterpret_cast<const uint8_t*>(s.data()), s.length())) {}

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

/// The non-owning read-only view to a ``Hello`` message returned by a zenoh entity as a reply to a "scout"
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

/// Owned string returned from zenoh. It is automatically freed when the object is destroyed.
class Str : public Owned<::z_owned_str_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Get the string value
    /// @return ``const char*`` null-terminated string pointer
    const char* c_str() const { return loan(); }

    /// @name Operators

    /// @brief Get the string value
    /// @return ``const char*`` null-terminated string pointer
    operator const char*() const { return loan(); }

    /// @brief Equality operator
    /// @param s the ``std::string_view`` to compare with
    /// @return true if the two strings are equal
    bool operator==(const std::string_view& s) const { return s == c_str(); }

    /// @brief Equality operator
    /// @param s the null-terminated string to compare with
    /// @return true if the two strings are equal
    bool operator==(const char* s) const { return std::string_view(s) == c_str(); }
};

class KeyExpr;
struct KeyExprView;

/// Empty type used to distinguish checked and unchecked constructing of KeyExprView
struct KeyExprUnchecked {
    /// @brief Constructs an instance of the type
    explicit KeyExprUnchecked() {}
};

/// @brief Make a string containing a key expression canonical.
/// @param s ``std::string`` with key expression
/// @param error error code returned if the key expression is invalid
/// @return true if the key expression was canonized, false otherwise
inline bool keyexpr_canonize(std::string& s, ErrNo& error);

/// @brief Make a string containing a key expression canonical.
/// @param s ``std::string`` with key expression
//// @return true if the key expression was canonized, false otherwise
inline bool keyexpr_canonize(std::string& s);

/// @brief Check if a string containing a key expression is canonical.
/// @param s ``std::string_view`` with key expression
/// @param error error code returned if the key expression is invalid
/// @return true if the key expression is canonical, false otherwise
inline bool keyexpr_is_canon(const std::string_view& s, ErrNo& error);

/// @brief Check if a string containing a key expression is canonical.
/// @param s ``std::string_view`` with key expression
/// @return true if the key expression is canonical, false otherwise
inline bool keyexpr_is_canon(const std::string_view& s);

#ifdef __ZENOHCXX_ZENOHC
/// @brief Concatenate the key expression and a string
/// @param k the key expression
/// @param s ``std::string_view`` representing a key expression
/// @return Newly allocated key expression ``zenoh::KeyExpr``
/// @note zenoh-c only
inline z::KeyExpr keyexpr_concat(const z::KeyExprView& k, const std::string_view& s);

/// @brief Join two key expressions, inserting a separator between them
/// @param a Key expression
/// @param b Key expressio
/// @return Newly allocated key expression ``zenoh::KeyExpr``
/// @note zenoh-c only
inline z::KeyExpr keyexpr_join(const z::KeyExprView& a, const z::KeyExprView& b);
#endif

/// @brief Checks if the two key expressions are equal
/// @param a Key expression
/// @param b Key expression
/// @param error Error code returned by ``::z_keyexpr_equals`` (value < -1 if any of the key expressions is not
/// valid)
/// @return true the key expressions are equal
inline bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

/// @brief Checks if the key expression includes another key expression, i.e. if the set defined by the key
/// expression contains the set defined by the other key expression
/// @param a Key expression
/// @param b Key expression
/// @param error Error code returned by ``::z_keyexpr_includes`` (value < -1 if any of the key expressions is not
/// valid)
/// @return true the key expression includes the other key expression
inline bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

/// @brief Checks if the key expression intersects with another key expression, i.e. there exists at least one key
/// which is contained in both of the sets defined by the key expressions
/// @param a Key expression
/// @param b Key expression
/// @param error Error code returned by ``::z_keyexpr_intersects`` (value < -1 if any of the key expressions is not
/// valid)
/// @return true the key expression intersects with the other key expression
inline bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

#ifdef __ZENOHCXX_ZENOHC
/// @brief Checks if the two key expressions are equal
/// @param a Key expression
/// @param b Key expression
/// @return true the key expressions are equal
inline bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b);

/// @brief Checks if the key expression includes another key expression, i.e. if the set defined by the key
/// expression contains the set defined by the other key expression
/// @param a Key expression
/// @param b Key expression
/// @return true the key expression includes the other key expression
inline bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b);

/// @brief Checks if the key expression intersects with another key expression, i.e. there exists at least one key
/// which is contained in both of the sets defined by the key expressions
/// @param a Key expression
/// @param b Key expression
/// @return true the key expression intersects with the other key expression
inline bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b);
#endif

/// The non-owning read-only view to a key expression in Zenoh.
/// See details about key expression syntax in the <a
/// href="https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md"> Key Expressions RFC</a>.
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
    KeyExprView(const char* name, z::KeyExprUnchecked) : Copyable(::z_keyexpr_unchecked(name)) {
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

#ifdef __ZENOHCXX_ZENOHPICO
    /// @brief Get string representation for key expression created with ``zenoh::Session::declare_keyexpr``
    /// @param s ``zenoh::Session`` object
    /// @return string representation of the key expression
    /// @note zenoh-pico only
    z::Str resolve(const z::Session& s) const;
#endif

#ifdef __ZENOHCXX_ZENOHC
    // operator += purposedly not defined to not provoke ambiguity between concat (which
    // mechanically connects strings) and join (which works with path elements)

    /// @brief see ``zenoh::keyexpr_concat``
    /// @note zenoh-c only
    z::KeyExpr concat(const std::string_view& s) const;

    /// @brief see ``zenoh::keyexpr_join``
    /// @note zenoh-c only
    z::KeyExpr join(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_equals``
    bool equals(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_includes``
    bool includes(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_intersects``
    bool intersects(const z::KeyExprView& v) const;
#endif

    /// @brief see ``zenoh::keyexpr_equals``
    bool equals(const z::KeyExprView& v, ErrNo& error) const;

    /// @brief see ``zenoh::keyexpr_includes``
    bool includes(const z::KeyExprView& v, ErrNo& error) const;

    /// @brief see ``zenoh::keyexpr_intersects``
    bool intersects(const z::KeyExprView& v, ErrNo& error) const;
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
/// The attachment.
/// A iteration driver based map of byte slice to byte slice.
struct AttachmentView : public Copyable<::z_attachment_t> {
    using Copyable::Copyable;

    /// The body of a loop over an attachment's key-value pairs.
    ///
    /// `key` and `value` are loaned to the body for the duration of a single call.
    ///
    /// Returning `true` is treated as `continue`.
    /// Returning `false` is treated as `break`.
    typedef std::function<bool(const BytesView& key, const BytesView& value)> IterBody;

    /// The driver of a loop over an attachment's key-value pairs.
    ///
    /// This function is expected to call `body` once for each key-value pair.
    /// Returning `false` value immediately (breaking iteration).
    typedef std::function<bool(const IterBody& body)> IterDriver;

    /// @name Constructors

    /// @brief AttachmentView constructor by interation driver
    AttachmentView(const AttachmentView::IterDriver& _iter_driver)
        : Copyable({static_cast<const void*>(&_iter_driver),
                    [](const void* data, z_attachment_iter_body_t body, void* ctx) -> int8_t {
                        const IterDriver* _iter_driver = static_cast<const IterDriver*>(data);
                        return (*_iter_driver)([&body, &ctx](const BytesView& key, const BytesView& value) {
                            return body(key, value, ctx);
                        });
                    }}) {}

    /// @brief AttachmentView constructor by the container which allows
    /// iterate by std::pair<std::string_view, std::string_view>
    /// (e.g. std::map<std::string_view, std::string_view>).
    template <typename T>
    AttachmentView(const T& pair_container)
        : Copyable({static_cast<const void*>(&pair_container),
                    [](const void* data, z_attachment_iter_body_t body, void* ctx) -> int8_t {
                        const T* pair_container = static_cast<const T*>(data);
                        for (const auto& it : *pair_container) {
                            int8_t ret = body(BytesView(it.first), BytesView(it.second), ctx);
                            if (ret) {
                                return ret;
                            }
                        }
                        return 0;
                    }}) {}

    /// @name Methods

    /// Returns the item value from the attachment by key
    /// @return the item value
    BytesView get(const BytesView& key) const { return ::z_attachment_get(*this, key); }

    /// Checks if the attachment is initialized
    /// @return true if the attachment is initialized
    bool check() const { return ::z_attachment_check(this); }

    /// Iterate over attachment's key-value pairs, breaking if `body` returns a `false` value for a key-value pair.
    /// `context` is passed to `body` to allow stateful closures.
    /// This function takes no ownership whatsoever.
    /// @return `true` if the iteration has passed to the end of all elements,
    /// 	or `false` if it has been interrupted
    bool iterate(const IterBody& body) const;

    /// @name Operators

    /// @brief Equality operator
    /// @param v other ``AttachmentView`` object
    /// @return true if the attachment objects encodings are equal
    bool operator==(const AttachmentView& v) const { return data == v.data && iteration_driver == v.iteration_driver; }

    /// @brief Inequality operator
    /// @param v other ``AttachmentView`` object
    /// @return true if the attachment objects are not equal
    bool operator!=(const AttachmentView& v) const { return !operator==(v); }
};

/// Reference to data buffer in shared memory with reference counting. When all instances of ``Payload`` are destroyed,
/// the buffer is freed.
/// It can be convenient if it's necessary to resend the buffer to one or multiple receivers without copying it.
/// This can be performed with ``Publisher::put_owned`` method.
///
/// @note zenoh-c only
class Payload : public Owned<::zc_owned_payload_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Clone reference to the payload buffer with incrementing it's reference count
    /// @return Reference to the payload buffer
    Payload rcinc() const { return Payload(::zc_payload_rcinc(&_0)); }

    /// @brief Access the data in the payload buffer
    /// @return ``BytesView`` object representing the data in the payload buffer
    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(_0.payload); }
};

/// Memory buffer returned by shared memory manager ``ShmManager``
///
/// @note zenoh-c only
class Shmbuf : public Owned<::zc_owned_shmbuf_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Returns the capacity of the SHM buffer in bytes
    /// @return capacity of the SHM buffer in bytes
    uintptr_t get_capacity() const { return ::zc_shmbuf_capacity(&_0); }

    /// @brief Returns the length of data in the SHM buffer in bytes
    /// @return length of data
    uintptr_t get_length() const { return ::zc_shmbuf_length(&_0); }

    /// @brief Set the length of data in the SHM buffer in bytes. Can't be greater than the capacity.
    /// @param length length of the data
    void set_length(uintptr_t length) { ::zc_shmbuf_set_length(&_0, length); }

    /// @brief Returns the payload object with the data from the SHM buffer. The ``Shmbuf`` object itself is invalidated
    /// @return ``Payload`` object with the data from the SHM buffer
    z::Payload into_payload() { return z::Payload(::zc_shmbuf_into_payload(&_0)); }

    /// @brief Returns the pointer to the data in the SHM buffer as ``uint8_t*``
    /// @return pointer to the data
    uint8_t* ptr() const { return ::zc_shmbuf_ptr(&_0); }

    /// @brief Returns the pointer to the data in the SHM buffer as ``char*``
    /// @return pointer to the data
    char* char_ptr() const { return reinterpret_cast<char*>(ptr()); }

    /// @brief Returns pointer to data and length of the data as ``std::string_view``
    /// @return ``std::string_view`` object representing the data in the SHM buffer
    std::string_view as_string_view() const {
        return std::string_view(reinterpret_cast<const char*>(ptr()), get_length());
    }
};

class ShmManager;
std::variant<z::ShmManager, z::ErrorMessage> shm_manager_new(const z::Session& session, const char* id, uintptr_t size);

/// Shared memory manager
///
/// @note zenoh-c only
class ShmManager : public Owned<::zc_owned_shm_manager_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a new shared memory manager. Allocate a new shared memory buffer or joins an existing one with the
    /// ``id``
    /// @param session ``Session`` object
    /// @param id string identifier of the shared memory manager
    /// @param size size of the shared memory buffer in bytes
    /// @return ``ShmManager`` object or ``zenoh::ErrorMessage`` if an error occurred
    friend std::variant<z::ShmManager, z::ErrorMessage> z::shm_manager_new(const z::Session& session, const char* id,
                                                                           uintptr_t size);

    /// @name Methods

    /// @brief Allocate a new shared memory buffer ``Shmbuf`` with the given capacity
    /// @param capacity capacity of buffer in bytes
    /// @return ``Shmbuf`` object or ``zenoh::ErrorMessage`` object if an error occurred
    std::variant<z::Shmbuf, z::ErrorMessage> alloc(uintptr_t capacity) const;

    /// @brief Perfrom defagmentation of the shared memory manager
    /// @return The amount of memory defragmented in bytes
    uintptr_t defrag() const { return ::zc_shm_defrag(&_0); }

    /// @brief Perform garbage collection of the shared memory manager
    /// @return The amount of memory freed in bytes
    uintptr_t gc() const { return ::zc_shm_gc(&_0); }

   private:
    ShmManager(const z::Session& session, const char* id, uintptr_t size);
};

#endif

/// A data sample.
///
/// A sample is the value associated to a given resource at a given point in time.
struct Sample : public Copyable<::z_sample_t> {
    using Copyable::Copyable;

    /// @name Methods

    /// @brief The resource key of this data sample.
    /// @return ``KeyExprView`` object representing the resource key
    const z::KeyExprView& get_keyexpr() const { return static_cast<const z::KeyExprView&>(keyexpr); }

    /// @brief The value of this data sample
    /// @return ``BytesView`` object representing the value
    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(payload); }

    /// @brief The encoding of the value of this data sample
    /// @return ``Encoding`` object
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }

    /// @brief The kind of this data sample (PUT or DELETE)
    /// @return ``zenoh::SampleKind`` value
    SampleKind get_kind() const { return kind; }

    /// @brief QoS settings this sample was sent with
    /// @return ``zenoh::QoS`` struct
    QoS get_qos() const { return qos; }

    /// @brief The timestamp of this data sample
    /// @return ``Timestamp`` object
    const z::Timestamp& get_timestamp() const { return static_cast<const z::Timestamp&>(timestamp); }
#ifdef __ZENOHCXX_ZENOHC

    /// @brief The payload object of the sample. If it represents a buffer in shared memory it can be
    /// resent without actually copying the data
    /// @return ``Payload`` object
    /// @note zenoh-c only
    z::Payload sample_payload_rcinc() const {
        return z::Payload(::zc_sample_payload_rcinc(static_cast<const ::z_sample_t*>(this)));
    }

    /// @brief The attachment of this data sample
    /// @return ``AttachmentView`` object
    const z::AttachmentView& get_attachment() const { return static_cast<const z::AttachmentView&>(attachment); }
#endif
};

/// A zenoh value. Contans refrence to data and it's encoding
struct Value : public Copyable<::z_value_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new value with the given payload and encoding
    /// @param payload ``BytesView`` object
    /// @param encoding ``Encoding`` value
    Value(const z::BytesView& payload, const z::Encoding& encoding) : Copyable({payload, encoding}) {}

    /// @brief Create a new value with the default encoding
    /// @param payload ``BytesView`` object
    Value(const z::BytesView& payload) : Value(payload, z::Encoding()) {}

    /// @brief Create a new value from null-terminated string with the default encoding
    /// @param payload null-terminated string
    Value(const char* payload) : Value(payload, z::Encoding()) {}

    /// @name Methods

    /// @brief The payload of this value
    /// @return ``BytesView`` object
    const z::BytesView& get_payload() const { return static_cast<const z::BytesView&>(payload); }

    /// @brief Set payload of this value
    /// @param _payload ``BytesView`` object
    /// @return referencew the value itself
    Value& set_payload(const z::BytesView& _payload) {
        payload = _payload;
        return *this;
    }

    /// @brief The encoding of this value
    /// @return ``Encoding`` object
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }

    /// @brief Set encoding of this value
    /// @param _encoding ``Encoding`` object
    /// @return referencew the value itself
    Value& set_encoding(const z::Encoding& _encoding) {
        encoding = _encoding;
        return *this;
    }

    /// @brief The payload of this value as a ``std::string_view``
    /// @return ``std::string_view`` object
    std::string_view as_string_view() const { return get_payload().as_string_view(); }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``Value`` to compare with
    /// @return true if the two values are equal (have the same payload and encoding)
    bool operator==(const Value& v) const {
        return get_payload() == v.get_payload() && get_encoding() == v.get_encoding();
    }

    /// @brief Inequality operator
    /// @param v the other ``Value`` to compare with
    /// @return true if the two values are not equal (have different payload or encoding)
    bool operator!=(const Value& v) const { return !operator==(v); }
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

    /// @brief Set the consolidation mode
    /// @param v ``zenoh::ConsolidationMode`` value
    QueryConsolidation& set_mode(ConsolidationMode v) {
        mode = v;
        return *this;
    }
    /// @brief The consolidation mode
    /// @return ``zenoh::ConsolidationMode`` value
    ConsolidationMode get_mode() const { return mode; }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``QueryConsolidation`` to compare with
    /// @return true if the two values are equal (have the same consolidation mode)
    bool operator==(const QueryConsolidation& v) const { return get_mode() == v.get_mode(); }

    /// @brief Inequality operator
    /// @param v the other ``QueryConsolidation`` to compare with
    /// @return true if the two values are not equal (have different consolidation mode)
    bool operator!=(const QueryConsolidation& v) const { return !operator==(v); }
};

/// Options passed to the get operation
struct GetOptions : public Copyable<::z_get_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``GetOptions`` value
    GetOptions() : Copyable(::z_get_options_default()) {}

    /// @name Methods

    /// @brief Set the target of the get operation
    /// @param v ``zenoh::QueryTarget`` value
    /// @return reference to the structure itself
    GetOptions& set_target(QueryTarget v) {
        target = v;
        return *this;
    }

    /// @brief Set the consolidation mode to apply on replies of get operation
    /// @param v ``QueryConsolidation`` value
    /// @return reference to the structure itself
    GetOptions& set_consolidation(z::QueryConsolidation v) {
        consolidation = v;
        return *this;
    }

    /// @brief Set an optional value to attach to the query
    /// @param v ``Value`` value
    /// @return reference to the structure itself
    GetOptions& set_value(z::Value v) {
        value = v;
        return *this;
    }

    /// @brief The target of the get operation
    /// @return ``zenoh::QueryTarget`` value
    QueryTarget get_target() const { return target; }

    /// @brief The consolidation mode to apply on replies of get operation
    /// @return ``QueryConsolidation`` value
    const z::QueryConsolidation& get_consolidation() const {
        return static_cast<const z::QueryConsolidation&>(consolidation);
    }

    /// @brief The optional value to attach to the query
    /// @return ``Value`` value
    const z::Value& get_value() const { return static_cast<const z::Value&>(value); }

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Set the timeout for the query operation
    /// @param v timeout in milliseconds. 0 means default query timeout from zenoh configuration.
    /// @return reference to the structure itself
    /// @note zenoh-c only
    GetOptions& set_timeout_ms(uint64_t ms) {
        timeout_ms = ms;
        return *this;
    }

    /// @brief The timeout for the query operation
    /// @return timeout in milliseconds. 0 means default query timeout from zenoh configuration.
    /// @note zenoh-c only
    uint64_t get_timeout_ms() const { return timeout_ms; }

    /// @brief Get the attachment
    /// @return ``zenoh::AttachmentView`` value
    const z::AttachmentView& get_attachment() const { return static_cast<const z::AttachmentView&>(attachment); }

    /// @brief Set the attachment
    /// @param a the ``zenoh::AttachmentView`` value
    /// @return reference to the structure itself
    GetOptions& set_attachment(const z::AttachmentView& a) {
        attachment = a;
        return *this;
    };
#endif

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``GetOptions`` to compare with
    /// @return true if the two values are equal (have the same target, consolidation mode and
    /// optional value)
    bool operator==(const GetOptions& v) const {
        return get_target() == v.get_target() && get_consolidation() == v.get_consolidation() &&
               get_value() == v.get_value()
#ifdef __ZENOHCXX_ZENOHC
               && get_timeout_ms() == v.get_timeout_ms()
#endif
            ;
    }

    /// @brief Inequality operator
    /// @param v the other ``GetOptions`` to compare with
    /// @return true if the two values are not equal (have different target, consolidation mode or
    /// optional value)
    bool operator!=(const GetOptions& v) const { return !operator==(v); }
};

/// Options passed to the put operation
struct PutOptions : public Copyable<::z_put_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``PutOptions`` value
    PutOptions() : Copyable(::z_put_options_default()) {}

    /// @name Methods

    /// @brief Get the encoding of the payload
    /// @return ``zenoh::Encoding`` value
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }

    /// @brief Set the encoding for the payload
    /// @param e ``zenoh::Encoding`` value
    /// @return reference to the structure itself
    PutOptions& set_encoding(z::Encoding e) {
        encoding = e;
        return *this;
    };

    /// @brief Get the congestion control mode
    /// @return ``zenoh::CongestionControl`` value
    CongestionControl get_congestion_control() const { return congestion_control; }

    /// @brief Set the congestion control mode
    /// @param v ``zenoh::CongestionControl`` value
    /// @return reference to the structure itself
    PutOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    };

    /// @brief Get the priority of the operation
    /// @return ``zenoh::Priority`` value
    Priority get_priority() const { return priority; }

    /// @brief Set the priority of the operation
    /// @param v ``zenoh::Priority`` value
    /// @return reference to the structure itself
    PutOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the attachment
    /// @return ``zenoh::AttachmentView`` value
    const z::AttachmentView& get_attachment() const { return static_cast<const z::AttachmentView&>(attachment); }

    /// @brief Set the attachment
    /// @param a the ``zenoh::AttachmentView`` value
    /// @return reference to the structure itself
    PutOptions& set_attachment(const z::AttachmentView& a) {
        attachment = a;
        return *this;
    };
#endif  // ifdef __ZENOHCXX_ZENOHC

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``PutOptions`` to compare with
    /// @return true if the two values are equal (have the same priority, congestion control mode
    /// and encoding)
    bool operator==(const PutOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control() &&
               get_encoding() == v.get_encoding();
    }

    /// @brief Inequality operator
    /// @param v the other ``PutOptions`` to compare with
    /// @return true if the two values are not equal (have different priority, congestion control
    /// mode or encoding)
    bool operator!=(const PutOptions& v) const { return !operator==(v); }
};

/// Options passed to the delete operation
struct DeleteOptions : public Copyable<::z_delete_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``DeleteOptions`` value
    DeleteOptions() : Copyable(::z_delete_options_default()) {}

    /// @name Methods

    /// @brief Get the congestion control mode
    /// @return ``zenoh::CongestionControl`` value
    CongestionControl get_congestion_control() const { return congestion_control; }

    /// @brief Set the congestion control mode
    /// @param v ``zenoh::CongestionControl`` value
    /// @return reference to the structure itself
    DeleteOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    }

    /// @brief Get the priority of the operation
    /// @return ``zenoh::Priority`` value
    Priority get_priority() const { return priority; }

    /// @brief Set the priority of the operation
    /// @param v ``zenoh::Priority`` value
    /// @return reference to the structure itself
    DeleteOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``DeleteOptions`` to compare with
    /// @return true if the two values are equal (have the same priority and congestion control)
    bool operator==(const DeleteOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control();
    }

    /// @brief Inequality operator
    /// @param v the other ``DeleteOptions`` to compare with
    /// @return true if the two values are not equal (have different priority or congestion control)
    bool operator!=(const DeleteOptions& v) const { return !operator==(v); }
};

/// Options passed to the ``Query::reply`` operation
struct QueryReplyOptions : public Copyable<::z_query_reply_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``QueryReplyOptions`` value
    QueryReplyOptions() : Copyable(::z_query_reply_options_default()) {}

    /// @name Methods

    /// @brief Get the encoding of the payload
    /// @return ``zenoh::Encoding`` value
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }

    /// @brief Set the encoding for the payload
    /// @param e ``zenoh::Encoding`` value
    /// @return reference to the structure itself
    QueryReplyOptions& set_encoding(z::Encoding e) {
        encoding = e;
        return *this;
    };

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the attachment
    /// @return ``zenoh::AttachmentView`` value
    const z::AttachmentView& get_attachment() const { return static_cast<const z::AttachmentView&>(attachment); }

    /// @brief Set the attachment
    /// @param a the ``zenoh::AttachmentView`` value
    /// @return reference to the structure itself
    QueryReplyOptions& set_attachment(const z::AttachmentView& a) {
        attachment = a;
        return *this;
    };
#endif  // ifdef __ZENOHCXX_ZENOHC

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``QueryReplyOptions`` to compare with
    /// @return true if the two values are equal (have the same encoding)
    bool operator==(const QueryReplyOptions& v) const { return get_encoding() == v.get_encoding(); }

    /// @brief Inequality operator
    /// @param v the other ``QueryReplyOptions`` to compare with
    /// @return true if the two values are not equal (have different encoding)
    bool operator!=(const QueryReplyOptions& v) const { return !operator==(v); }
};

/// The query to be answered by a ``Queryable``
class Query : public Copyable<::z_query_t> {
   public:
    using Copyable::Copyable;

    /// @name Methods

    /// @brief Get the key expression of the query
    /// @return ``zenoh::KeyExprView`` value
    z::KeyExprView get_keyexpr() const { return z::KeyExprView(::z_query_keyexpr(this)); }

    /// @brief Get a query's <a href=https://github.com/eclipse-zenoh/roadmap/tree/main/rfcs/ALL/Selectors>value
    /// selector</a>
    /// @return ``zenoh::BytesView`` value
    z::BytesView get_parameters() const { return z::BytesView(::z_query_parameters(this)); }

    /// @brief Get the value of the query
    /// @return ``zenoh::Value`` value
    z::Value get_value() const { return z::Value(::z_query_value(this)); }

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the attachment of the query
    /// @return ``zenoh::AttachmentView`` value
    z::AttachmentView get_attachment() const { return z::AttachmentView(::z_query_attachment(this)); }
#endif  // ifdef __ZENOHCXX_ZENOHC

    /// @brief Send reply to the query
    /// @param key the ``KeyExprView`` of the ``Queryable``. **NOT** the key expression from the ``Query::get_keyexpr``.
    /// E.g. queryable registered as "foo/bar" gets query with key expression "foo/*" and replies with key expression
    /// "foo/bar"
    /// This function can be called multiple times to send multiple replies to the same query. The reply
    /// will be considered complete when the ``Queryable`` callback returns.
    /// @param payload the ``BytesView`` with payload to be sent
    /// @param options the ``QueryReplyOptions`` to be used for the reply
    /// @param error the ``zenoh::ErrNo`` error code
    /// @return true if the reply was sent successfully
    bool reply(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions& options,
               ErrNo& error) const;

    /// @brief Send reply to the query
    /// @param key the ``KeyExprView`` of the queryable
    /// @param payload the ``BytesView`` with payload to be sent
    /// @param options the ``QueryReplyOptions`` to be used for the reply
    /// @return true if the reply was sent successfully
    bool reply(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions& options) const;

    /// @brief Send reply to the query
    /// @param key the ``KeyExprView`` of the queryable
    /// @param payload the ``BytesView`` with payload to be sent
    /// @param error the ``zenoh::ErrNo`` error code
    /// @return true if the reply was sent successfully
    bool reply(z::KeyExprView key, const z::BytesView& payload, ErrNo& error) const;

    /// @brief Send reply to the query
    /// @param key the ``KeyExprView`` of the queryable
    /// @param payload the ``BytesView`` with payload to be sent
    /// @return true if the reply was sent successfully
    bool reply(z::KeyExprView key, const z::BytesView& payload) const;

   private:
    bool reply_impl(z::KeyExprView key, const z::BytesView& payload, const z::QueryReplyOptions* options,
                    ErrNo& error) const;
};

/// Options to be passed when declaring a ``Queryable``
struct QueryableOptions : public Copyable<::z_queryable_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``QueryableOptions`` value
    QueryableOptions() : Copyable(::z_queryable_options_default()) {}

    /// @name Methods

    /// @brief Get the completeness of the queryable
    /// A queryable is defined as complete if it can serve any query “completely” related to the key expression on which
    /// it serves. The querier will not benefit from extra information from any other queryable.
    /// @return true if the queryable is complete
    bool get_complete() const { return complete; }

    /// @brief Set the completeness of the queryable
    /// @param v true if the queryable is complete
    /// @return reference to the structure itself
    QueryableOptions& set_complete(bool v) {
        complete = v;
        return *this;
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``QueryableOptions`` to compare with
    /// @return true if the two values are equal (have the same completeness)
    bool operator==(const QueryableOptions& v) const { return get_complete() == v.get_complete(); }

    /// @brief Inequality operator
    /// @param v the other ``QueryableOptions`` to compare with
    /// @return true if the two values are not equal (have different completeness)
    bool operator!=(const QueryableOptions& v) const { return !operator==(v); }
};

/// Options to be passed when declaring a ``'Subscriber``
struct SubscriberOptions : public Copyable<::z_subscriber_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``SubscriberOptions`` value
    SubscriberOptions() : Copyable(::z_subscriber_options_default()) {}

    /// @name Methods

    /// @brief Get the reliability of the subscriber
    /// @return ``zenoh::Reliability`` value
    Reliability get_reliability() const { return reliability; }

    /// @brief Set the reliability of the subscriber
    /// @param v the ``zenoh::Reliability`` value
    /// @return reference to the structure itself
    SubscriberOptions& set_reliability(Reliability v) {
        reliability = v;
        return *this;
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``SubscriberOptions`` to compare with
    /// @return true if the two values are equal (have the same reliability)
    bool operator==(const SubscriberOptions& v) const { return get_reliability() == v.get_reliability(); }

    /// @brief Inequality operator
    /// @param v the other ``SubscriberOptions`` to compare with
    /// @return true if the two values are not equal (have different reliability)
    bool operator!=(const SubscriberOptions& v) const { return !operator==(v); }
};

/// Options to be passed when declaring a ``PullSubscriber``
struct PullSubscriberOptions : public Copyable<::z_pull_subscriber_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``PullSubscriberOptions`` value
    PullSubscriberOptions() : Copyable(::z_pull_subscriber_options_default()) {}

    /// @name Methods

    /// @brief Get the reliability of the pull subscriber
    /// @return ``zenoh::Reliability`` value
    Reliability get_reliability() const { return reliability; }

    /// @brief Set the reliability of the pull subscriber
    /// @param v the ``zenoh::Reliability`` value
    /// @return reference to the structure itself
    PullSubscriberOptions& set_reliability(Reliability v) {
        reliability = v;
        return *this;
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``PullSubscriberOptions`` to compare with
    /// @return true if the two values are equal (have the same reliability)
    bool operator==(const PullSubscriberOptions& v) const { return get_reliability() == v.get_reliability(); }

    /// @brief Inequality operator
    /// @param v the other ``PullSubscriberOptions`` to compare with
    /// @return true if the two values are not equal (have different reliability)
    bool operator!=(const PullSubscriberOptions& v) const { return !operator==(v); }
};

/// Options to be passed when declaring a ``Publisher``
struct PublisherOptions : public Copyable<::z_publisher_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``PublisherOptions`` value
    PublisherOptions() : Copyable(::z_publisher_options_default()) {}

    /// @name Methods

    /// @brief Get the congestion control of the publisher
    /// @return ``zenoh::CongestionControl`` value
    CongestionControl get_congestion_control() const { return congestion_control; }

    /// @brief Set the congestion control of the publisher
    /// @param v the ``zenoh::CongestionControl`` value
    /// @return reference to the structure itself
    PublisherOptions& set_congestion_control(CongestionControl v) {
        congestion_control = v;
        return *this;
    }

    /// @brief Get the priority of the publisher
    /// @return ``zenoh::Priority`` value
    Priority get_priority() const { return priority; }

    /// @brief Set the priority of the publisher
    /// @param v the ``zenoh::Priority`` value
    /// @return reference to the structure itself
    PublisherOptions& set_priority(Priority v) {
        priority = v;
        return *this;
    }

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``PublisherOptions`` to compare with
    /// @return true if the two values are equal (have the same congestion control and priority)
    bool operator==(const PublisherOptions& v) const {
        return get_priority() == v.get_priority() && get_congestion_control() == v.get_congestion_control();
    }

    /// @brief Inequality operator
    /// @param v the other ``PublisherOptions`` to compare with
    /// @return true if the two values are not equal (have different congestion control or priority)
    bool operator!=(const PublisherOptions& v) const { return !operator==(v); }
};

/// Options to be passed to ``Publisher::put`` operation
struct PublisherPutOptions : public Copyable<::z_publisher_put_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create a new default ``PublisherPutOptions`` value
    PublisherPutOptions() : Copyable(::z_publisher_put_options_default()) {}

    /// @name Methods

    /// @brief Get the encoding of the publisher
    /// @return ``zenoh::Encoding`` value
    const z::Encoding& get_encoding() const { return static_cast<const z::Encoding&>(encoding); }

    /// @brief Set the encoding of the publisher
    /// @param e the ``zenoh::Encoding`` value
    /// @return reference to the structure itself
    PublisherPutOptions& set_encoding(z::Encoding e) {
        encoding = e;
        return *this;
    };

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the attachment
    /// @return ``zenoh::AttachmentView`` value
    const z::AttachmentView& get_attachment() const { return static_cast<const z::AttachmentView&>(attachment); }

    /// @brief Set the attachment
    /// @param a the ``zenoh::AttachmentView`` value
    /// @return reference to the structure itself
    PublisherPutOptions& set_attachment(const z::AttachmentView& a) {
        attachment = a;
        return *this;
    };
#endif  // ifdef __ZENOHCXX_ZENOHC

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``PublisherPutOptions`` to compare with
    /// @return true if the two values are equal (have the same encoding)
    bool operator==(const PublisherPutOptions& v) const { return get_encoding() == v.get_encoding(); }

    /// @brief Inequality operator
    /// @param v the other ``PublisherPutOptions`` to compare with
    /// @return true if the two values are not equal (have different encoding)
    bool operator!=(const PublisherPutOptions& v) const { return !operator==(v); }
};

/// Options to be passed to delete operation of a publisher
struct PublisherDeleteOptions : public Copyable<::z_publisher_delete_options_t> {
    using Copyable::Copyable;

    /// @name Constructors

    PublisherDeleteOptions() : Copyable(::z_publisher_delete_options_default()) {}

    /// @name Operators

    /// @brief Equality operator
    /// @param v the other ``PublisherDeleteOptions`` to compare with
    /// @return true if the two values are equal
    bool operator==(const z::PublisherOptions&) const { return true; }

    /// @brief Inequality operator
    /// @param v the other ``PublisherDeleteOptions`` to compare with
    /// @return true if the two values are not equal
    bool operator!=(const z::PublisherOptions& v) const { return !operator==(v); }
};

/// Owned key expression.
/// See details about key expression syntax in the <a
/// href="https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md"> Key Expressions RFC</a>.
class KeyExpr : public Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create an uninitialized instance
    explicit KeyExpr(nullptr_t) : Owned(nullptr) {}

    /// @brief Create a new instance from a null-terminated string
    explicit KeyExpr(const char* name) : Owned(::z_keyexpr_new(name)) {}

    /// @name Methods

    /// @brief Get the key expression value
    /// @return ``KeyExprView`` referencing the key expression value in the object
    z::KeyExprView as_keyexpr_view() const { return z::KeyExprView(loan()); }

    /// @brief Get the key expression value
    /// @return ``BytesView`` referencing the key expression value in the object
    z::BytesView as_bytes() const { return as_keyexpr_view().as_bytes(); }

    /// @brief Get the key expression value
    /// @return ``std::string_view`` referencing the key expression value in the object
    std::string_view as_string_view() const { return as_keyexpr_view().as_string_view(); }

    /// @name Operators

    /// @brief Get the key expression value
    /// @return ``KeyExprView`` referencing the key expression value in the object
    operator z::KeyExprView() const { return as_keyexpr_view(); }

    /// @brief Equality operator
    /// @param v the ``std::string_view`` to compare with
    /// @return true if the key expression is equal to the string
    bool operator==(const std::string_view& v) { return as_string_view() == v; }

#ifdef __ZENOHCXX_ZENOHC
    /// @brief see ``zenoh::keyexpr_concat``
    /// @note zenoh-c only
    z::KeyExpr concat(const std::string_view& s) const;

    /// @brief see ``zenoh::keyexpr_join``
    /// @note zenoh-c only
    z::KeyExpr join(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_equals``
    bool equals(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_includes``
    bool includes(const z::KeyExprView& v) const;

    /// @brief see ``zenoh::keyexpr_intersects``
    bool intersects(const z::KeyExprView& v) const;
#endif

    /// @brief see ``zenoh::keyexpr_equals``
    bool equals(const z::KeyExprView& v, ErrNo& error) const;

    /// @brief see ``zenoh::keyexpr_includes``
    bool includes(const z::KeyExprView& v, ErrNo& error) const;

    /// @brief see ``zenoh::keyexpr_intersects``
    bool intersects(const z::KeyExprView& v, ErrNo& error) const;
};

class ScoutingConfig;

/// Zenoh config
class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create the default configuration
    Config() : Owned(::z_config_default()) {}
#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get config parameter by the string key
    /// @param key the key
    /// @return the ``Str`` value of the config parameter
    /// @note zenoh-c only
    z::Str get(const char* key) const { return z::Str(::zc_config_get(loan(), key)); }

    /// @brief Get the whole config as a JSON string
    /// @return the JSON string in ``Str``
    /// @note zenoh-c only
    z::Str to_string() const { return z::Str(::zc_config_to_string(loan())); }

    /// @brief Insert a config parameter by the string key
    /// @param key the key
    /// @param value the JSON string value
    /// @return true if the parameter was inserted
    /// @note zenoh-c only
    bool insert_json(const char* key, const char* value) { return ::zc_config_insert_json(loan(), key, value) == 0; }
#endif
#ifdef __ZENOHCXX_ZENOHPICO
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
    /// @brief Create ``ScoutingConfig`` from the config
    /// @return the ``ScoutingConfig`` object
    z::ScoutingConfig create_scouting_config();
};

#ifdef __ZENOHCXX_ZENOHC

/// @brief Create the default configuration for "peer" mode
/// @return the ``Config`` object
/// @note zenoh-c only
inline z::Config config_peer() { return z::Config(::z_config_peer()); }

/// @brief Create the configuration from the JSON file
/// @param path path to the file
/// @return the ``Config`` object
/// @note zenoh-c only
std::variant<z::Config, ErrorMessage> config_from_file(const char* path);

/// @brief Create the configuration from the JSON string
/// @param s the JSON string
/// @return the ``Config`` object
/// @note zenoh-c only
std::variant<z::Config, ErrorMessage> config_from_str(const char* s);

/// @brief Create the configuration for "client" mode
/// @param peers the array of peers
/// @return the ``Config`` object
/// @note zenoh-c only
std::variant<z::Config, ErrorMessage> config_client(const z::StrArrayView& peers);

/// @brief Create the configuration for "client" mode
/// @param peers the array of peers
/// @return the ``Config`` object
/// @note zenoh-c only
std::variant<z::Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers);
#endif

/// An owned reply from queryable to ``Session::get`` operation
class Reply : public Owned<::z_owned_reply_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Check if the reply is OK
    /// @return true if the reply is OK
    bool is_ok() const { return ::z_reply_is_ok(&_0); }

    /// @brief Get the reply value
    /// @return the ``Sample`` value of the reply if reply is OK, otherwise ``zenoh::ErrorMessage``
    std::variant<z::Sample, ErrorMessage> get() const {
        if (is_ok()) {
            return z::Sample{::z_reply_ok(&_0)};
        } else {
            return z::ErrorMessage{::z_reply_err(&_0)};
        }
    }
};

/// An owned zenoh subscriber. Destroying subscriber cancels the subscription
/// Constructed by ``Session::declare_subscriber`` method
class Subscriber : public Owned<::z_owned_subscriber_t> {
   public:
    using Owned::Owned;

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the subscriber
    /// @return ``zenoh::KeyExpr`` value
    z::KeyExpr get_keyexpr() const { return ::z_subscriber_keyexpr(loan()); }
#endif
};

/// An owned zenoh pull subscriber. Destroying the subscriber cancels the subscription.
/// Constructed by ``Session::declare_pull_subscriber`` method
class PullSubscriber : public Owned<::z_owned_pull_subscriber_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Pull the next sample
    /// @return true if the sample was pulled, false otherwise
    bool pull() { return ::z_subscriber_pull(loan()) == 0; }

    /// @brief Pull the next sample
    /// @param error the error code
    /// @return true if the sample was pulled, false otherwise
    bool pull(ErrNo& error) {
        error = ::z_subscriber_pull(loan());
        return error == 0;
    }
};

/// An owned zenoh queryable. Constructed by ``Session::declare_queryable`` method
class Queryable : public Owned<::z_owned_queryable_t> {
   public:
    using Owned::Owned;
};

/// An owned zenoh publisher. Constructed by ``Session::declare_publisher`` method
class Publisher : public Owned<::z_owned_publisher_t> {
   public:
    using Owned::Owned;

    /// @name Methods

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @param options ``PublisherPutOptions``
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the payload was published, false otherwise
    bool put(const z::BytesView& payload, const z::PublisherPutOptions& options, ErrNo& error);

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the payload was published, false otherwise
    bool put(const z::BytesView& payload, ErrNo& error);

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @param options ``PublisherPutOptions``
    /// @return true if the payload was published, false otherwise
    bool put(const z::BytesView& payload, const z::PublisherPutOptions& options);

    /// @brief Publish the payload
    /// @param payload ``Payload`` to publish
    /// @return true if the payload was published, false otherwise
    bool put(const z::BytesView& payload);

    /// @brief Undeclare the resource
    /// @param options ``PublisherDeleteOptions``
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the request was sent, false otherwise
    bool delete_resource(const z::PublisherDeleteOptions& options, ErrNo& error);

    /// @brief Undeclare the resource
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the request was sent, false otherwise
    bool delete_resource(ErrNo& error);

    /// @brief Undeclare the resource
    /// @param options ``PublisherDeleteOptions``
    /// @return true if the request was sent, false otherwise
    bool delete_resource(const z::PublisherDeleteOptions& options);

    /// @brief Send a delete request
    /// @return true if the request was sent, false otherwise
    bool delete_resource();

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the publisher
    /// @return ``zenoh::KeyExpr`` value
    z::KeyExpr get_keyexpr() const { return ::z_publisher_keyexpr(loan()); }
#endif

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Publish the payload got from ``ShmBuf`` or from ``Sample``
    /// @param payload ``Payload`` to publish
    /// @param options ``PublisherPutOptions``
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the payload was published, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::Payload&& payload, const z::PublisherPutOptions& options, ErrNo& error);

    /// @brief Publish the payload got from ``ShmBuf`` or from ``Sample``
    /// @param payload ``Payload`` to publish
    /// @param error the error code ``zenoh::ErrNo``
    /// @return true if the payload was published, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::Payload&& payload, ErrNo& error);

    /// @brief Publish the payload got from ``ShmBuf`` or from ``Sample``
    /// @param payload ``Payload`` to publish
    /// @param options ``PublisherPutOptions``
    /// @return true if the payload was published, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::Payload&& payload, const z::PublisherPutOptions& options);

    /// @brief Publish the payload got from ``ShmBuf`` or from ``Sample``
    /// @param payload ``Payload`` to publish
    /// @return true if the payload was published, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::Payload&& payload);
#endif

   private:
    bool put_impl(const z::BytesView& payload, const z::PublisherPutOptions* options, ErrNo& error);
    bool delete_impl(const z::PublisherDeleteOptions* options, ErrNo& error);
#ifdef __ZENOHCXX_ZENOHC
    bool put_owned_impl(z::Payload&& payload, const z::PublisherPutOptions* options, ErrNo& error);
#endif
};

/// A zenoh-allocated hello message returned by a zenoh entity to a scout message sent with scout operation
class Hello : public Owned<::z_owned_hello_t> {
   public:
    using Owned::Owned;

    /// @name Operators

    /// @brief Get the content of the hello message
    /// @return the content of the hello message as ``HelloView``
    operator z::HelloView() const { return z::HelloView(loan()); }
};

/// @brief Callback type passed to ``Session::get``to process ``Reply``s from ``Queryable``s
typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, z::Reply> ClosureReply;

/// @brief Callback type passed to ``Session::declare_queryable`` to process received ``Query``s
typedef ClosureConstRefParam<::z_owned_closure_query_t, ::z_query_t, z::Query> ClosureQuery;

/// @brief Callback type passed to ``Session::declare_subscriber`` to process received ``Sample``s
typedef ClosureConstRefParam<::z_owned_closure_sample_t, ::z_sample_t, z::Sample> ClosureSample;

/// @brief Callback type passed to ``Session::info_routers_zid`` and ``Session::info_peers_zid`` to process received
/// ``Id``s
typedef ClosureConstRefParam<::z_owned_closure_zid_t, ::z_id_t, z::Id> ClosureZid;

/// @brief Callback type passed to ``zenoh::scout`` to process received ``Hello``s
typedef ClosureMoveParam<::z_owned_closure_hello_t, ::z_owned_hello_t, z::Hello> ClosureHello;

/// @brief Configuration for a ``zenoh::scout`` operation. This configuration can be created by
/// ``Config::create_scouting_config`` method.
class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a default ``ScoutingConfig``
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
};

/// @brief Scout for zenoh entities in the network
/// @param config ``ScoutingConfig`` to use
/// @param callback ``zenoh::ClosureHello`` to process received ``Hello``s
/// @param error ``zenoh:::ErrNo`` to store the error code
/// @return true if the scout operation was successful, false otherwise
bool scout(z::ScoutingConfig&& config, z::ClosureHello&& callback, ErrNo& error);

/// @brief Scout for zenoh entities in the network
/// @param config ``ScoutingConfig`` to use
/// @param callback ``zenoh::ClosureHello`` to process received ``Hello``s
/// @return true if the scout operation was successful, false otherwise
bool scout(z::ScoutingConfig&& config, z::ClosureHello&& callback);

class Session;

/// @brief Create a ``Session`` with the given ``Config``
/// @param config ``Config`` to use
/// @param start_background_tasks for zenoh-pico only. If true, start background threads which handles the network
/// traffic. If false, the threads should be called manually with ``Session::start_read_task`` and
/// ``Session::start_lease_task`` or methods ``Session::read``, ``Session::send_keep_alive`` and
/// ``Session::send_join`` should be called in loop.
/// @return a ``Session`` if the session was successfully created, an ``zenoh::ErrorMessage`` otherwise
std::variant<z::Session, z::ErrorMessage> open(z::Config&& config, bool start_background_tasks = true);

/// Main zenoh session class.
class Session : public Owned<::z_owned_session_t> {
   public:
    using Owned::Owned;

#ifdef __ZENOHCXX_ZENOHPICO
    //
    // For zenoh-pico ``Session`` may start background tasks on creation and stop them in destructor.
    // So for zenoh-pico ``Session`` overrides destructor and ``drop`` method.
    // This also makes necessary to define move constructor and move assignment operator, which are not automatically
    // generated by the compiler anymore
    //
    Session(Session&& other) : Owned(std::move(other)) {}
    Session(nullptr_t) : Owned(nullptr) {}
    Session&& operator=(Session&& other);
    void drop();
    ~Session() { drop(); }
#endif

    friend std::variant<z::Session, z::ErrorMessage> z::open(z::Config&& config, bool start_background_tasks);

    /// @name Methods

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Clone the ``Session`` instance incrementing the reference count
    /// This is possible in zenoh-c only where ``Session`` is a reference counted object.
    /// @return a new ``Session`` instance
    /// @note zenoh-c only
    Session rcinc() { return Session(::zc_session_rcinc(loan())); }
#endif

    /// @brief Get the unique identifier of the zenoh node associated to this ``Session``
    /// @return the unique identifier ``Id``
    z::Id info_zid() const { return ::z_info_zid(loan()); }

    /// @brief Create ``KeyExpr`` instance with numeric id registered in ``Session`` routing tables
    /// @param keyexpr ``KeyExprView`` representing string key expression
    /// @return ``KeyExpr`` instance
    z::KeyExpr declare_keyexpr(const z::KeyExprView& keyexpr);

    /// @brief Remove ``KeyExpr`` instance from ``Session`` and drop ``KeyExpr`` instance
    /// @param keyexpr ``KeyExpr`` instance to drop
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the ``KeyExpr`` instance was successfully removed, false otherwise
    bool undeclare_keyexpr(z::KeyExpr&& keyexpr, ErrNo& error);

    /// @brief Remove ``KeyExpr`` instance from ``Session`` and drop ``KeyExpr`` instance
    /// @param keyexpr ``KeyExpr`` instance to drop
    /// @return true if the ``KeyExpr`` instance was successfully removed, false otherwise
    bool undeclare_keyexpr(z::KeyExpr&& keyexpr);

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param keyexpr ``KeyExprView`` the key expression matching resources to query
    /// @param parameters the null-terminated string of query parameters
    /// @param callback ``zenoh::ClosureReply`` callback for process ``Reply``s
    /// @param options ``GetOptions`` query options
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the query operation was successful, false otherwise
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, const z::GetOptions& options,
             ErrNo& error);

    /// @brief Query data from the matching queryables in the system. Replies are provided through a callback function.
    /// @param keyexpr ``KeyExprView`` the key expression matching resources to query
    /// @param parameters the null-terminated string of query parameters
    /// @param callback ``zenoh::ClosureReply`` callback for process ``Reply``s
    /// @param options ``GetOptions`` query options
    /// @return true if the query operation was successful, false otherwise
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, const z::GetOptions& options);

    /// @brief Query data from the matching queryables in the system.
    /// @param keyexpr ``KeyExprView`` the key expression matching resources to query
    /// @param parameters the null-terminated string of query parameters
    /// @param callback ``zenoh::ClosureReply`` callback for process ``Reply``s
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the query operation was successful, false otherwise
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback, ErrNo& error);

    /// @brief Query data from the matching queryables in the system.
    /// @param keyexpr ``KeyExprView`` the key expression matching resources to query
    /// @param parameters the null-terminated string of query parameters
    /// @param callback ``zenoh::ClosureReply`` callback for process ``Reply``s
    /// @return true if the query operation was successful, false otherwise
    bool get(z::KeyExprView keyexpr, const char* parameters, z::ClosureReply&& callback);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``BytesView`` the data to publish
    /// @param options ``PutOptions`` put options
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the publish operation was successful, false otherwise
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, const z::PutOptions& options, ErrNo& error);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``BytesView`` the data to publish
    /// @param options ``PutOptions`` put options
    /// @return true if the publish operation was successful, false otherwise
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, const z::PutOptions& options);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``BytesView`` the data to publish
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the publish operation was successful, false otherwise
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload, ErrNo& error);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``BytesView`` the data to publish
    /// @return true if the publish operation was successful, false otherwise
    bool put(z::KeyExprView keyexpr, const z::BytesView& payload);

    /// @brief Undeclare a resource. Equal to ``Publisher::delete_resource``
    /// @param keyexpr ``KeyExprView`` the key expression to delete the resource
    /// @param options ``DeleteOptions`` delete options
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the undeclare operation was successful, false otherwise
    bool delete_resource(z::KeyExprView keyexpr, const z::DeleteOptions& options, ErrNo& error);

    /// @brief Undeclare a resource. Equal to ``Publisher::delete_resource``
    /// @param keyexpr ``KeyExprView`` the key expression to delete the resource
    /// @param options ``DeleteOptions`` delete options
    /// @return true if the undeclare operation was successful, false otherwise
    bool delete_resource(z::KeyExprView keyexpr, const z::DeleteOptions& options);

    /// @brief Undeclare a resource. Equal to ``Publisher::delete_resource``
    /// @param keyexpr ``KeyExprView`` the key expression to delete the resource
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the undeclare operation was successful, false otherwise
    bool delete_resource(z::KeyExprView keyexpr, ErrNo& error);

    /// @brief Undeclare a resource.
    /// @param keyexpr ``KeyExprView`` the key expression to delete the resource
    /// @return true if the undeclare operation was successful, false otherwise
    bool delete_resource(z::KeyExprView keyexpr);

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``Payload`` the data to publish
    /// @param options ``PutOptions`` put options
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the publish operation was successful, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const z::PutOptions& options, ErrNo& error);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``Payload`` the data to publish
    /// @param options ``PutOptions`` put options
    /// @return true if the publish operation was successful, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const z::PutOptions& options);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``Payload`` the data to publish
    /// @param error ``zenoh::ErrNo`` to store the error code
    /// @return true if the publish operation was successful, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload, ErrNo& error);

    /// @brief Publish data to the matching subscribers in the system. Equal to ``Publisher::put_owned``
    /// @param keyexpr ``KeyExprView`` the key expression to put the data
    /// @param payload ``Payload`` the data to publish
    /// @return true if the publish operation was successful, false otherwise
    /// @note zenoh-c only
    bool put_owned(z::KeyExprView keyexpr, z::Payload&& payload);
#endif

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the ``Session::get`` requests
    /// @param callback ``zenoh::ClosureQuery`` the callback to handle ``Query`` requests
    /// @param options ``QueryableOptions`` queryable options
    /// @return a ``Queryable`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Queryable, ErrorMessage> declare_queryable(z::KeyExprView keyexpr, z::ClosureQuery&& callback,
                                                               const z::QueryableOptions& options);

    /// @brief Create a ``Queryable`` object to answer to ``Session::get`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the ``Session::get`` requests
    /// @param callback ``zenoh::ClosureQuery`` the callback to handle ``Query`` requests
    /// @return a ``Queryable`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Queryable, ErrorMessage> declare_queryable(z::KeyExprView keyexpr, z::ClosureQuery&& callback);

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put``, ``Session::delete_resource`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the publishers
    /// @param callback ``zenoh::ClosureSample`` the callback to handle the received ``Sample`` objects
    /// @param options ``SubscriberOptions`` subscriber options
    /// @return a ``Subscriber`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Subscriber, ErrorMessage> declare_subscriber(z::KeyExprView keyexpr, z::ClosureSample&& callback,
                                                                 const z::SubscriberOptions& options);

    /// @brief Create a ``Subscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put``, ``Session::delete_resource`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the publishers
    /// @param callback ``zenoh::ClosureSample`` the callback to handle the received ``Sample`` objects
    /// @return a ``Subscriber`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Subscriber, ErrorMessage> declare_subscriber(z::KeyExprView keyexpr, z::ClosureSample&& callback);

    /// @brief Create a ``PullSubscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put``, ``Session::delete_resource`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the publishers
    /// @param callback ``zenoh::ClosureSample`` the callback to handle the received ``Sample`` objects
    /// @param options ``PullSubscriberOptions`` pull subscriber options
    /// @return a ``PullSubscriber`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::PullSubscriber, ErrorMessage> declare_pull_subscriber(z::KeyExprView keyexpr,
                                                                          z::ClosureSample&& callback,
                                                                          const z::PullSubscriberOptions& options);

    /// @brief Create a ``PullSubscriber`` object to receive data from matching ``Publisher`` objects or from
    /// ``Session::put``, ``Session::delete_resource`` requests
    /// @param keyexpr ``KeyExprView`` the key expression to match the publishers
    /// @param callback ``zenoh::ClosureSample`` the callback to handle the received ``Sample`` objects
    /// @return a ``PullSubscriber`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::PullSubscriber, ErrorMessage> declare_pull_subscriber(z::KeyExprView keyexpr,
                                                                          z::ClosureSample&& callback);

    /// @brief Create a ``Publisher`` object to publish data to matching ``Subscriber`` and ``PullSubscriber`` objects
    /// @param keyexpr ``KeyExprView`` the key expression to match the subscribers
    /// @param options ``PublisherOptions`` publisher options
    /// @return a ``Publisher`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Publisher, ErrorMessage> declare_publisher(z::KeyExprView keyexpr,
                                                               const z::PublisherOptions& options);

    /// @brief Create a ``Publisher`` object to publish data to matching ``Subscriber`` and ``PullSubscriber`` objects
    /// @param keyexpr ``KeyExprView`` the key expression to match the subscribers
    /// @return a ``Publisher`` object if the declaration was successful, an ``zenoh::ErrorMessage`` otherwise
    std::variant<z::Publisher, ErrorMessage> declare_publisher(z::KeyExprView keyexpr);

    /// @brief Fetches the Zenoh IDs of all connected routers.
    /// @param callback ``zenoh::ClosureZid`` the callback to handle the received ``Id`` objects. Callback is
    /// guaranteed to never be called concurrently, and is guaranteed to be dropped before this function exits.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    bool info_routers_zid(z::ClosureZid&& callback, ErrNo& error);

    /// @brief Fetches the Zenoh IDs of all connected routers.
    /// @param callback ``zenoh::ClosureZid`` the callback to handle the received ``Id`` objects. Callback is
    /// guaranteed to never be called concurrently, and is guaranteed to be dropped before this function exits.
    /// @return true if the operation was successful, false otherwise
    bool info_routers_zid(z::ClosureZid&& callback);

    /// @brief Fetches the Zenoh IDs of all connected peers.
    /// @param callback ``zenoh::ClosureZid`` the callback to handle the received ``Id`` objects. Callback is
    /// guaranteed to never be called concurrently, and is guaranteed to be dropped before this function exits.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    bool info_peers_zid(z::ClosureZid&& callback, ErrNo& error);

    /// @brief Fetches the Zenoh IDs of all connected peers.
    /// @param callback ``zenoh::ClosureZid`` the callback to handle the received ``Id`` objects. Callback is
    /// guaranteed to never be called concurrently, and is guaranteed to be dropped before this function exits.
    /// @return true if the operation was successful, false otherwise
    bool info_peers_zid(z::ClosureZid&& callback);

#ifdef __ZENOHCXX_ZENOHPICO

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task();

    /// @brief Start a separate task to read from the network and process the messages as soon as they are received.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool start_read_task(ErrNo& error);

    /// @brief Stop the read task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task();

    /// @brief Stop the read task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_read_task(ErrNo& error);

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
    bool start_lease_task(ErrNo& error);

    /// @brief Stop the lease task
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task();

    /// @brief Stop the lease task
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the operation was successful, false otherwise
    /// @note zenoh-pico only
    bool stop_lease_task(ErrNo& error);

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
    bool read(ErrNo& error);

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive();

    /// @brief Triggers a single execution of keep alive procedure. It will send KeepAlive messages when needed and
    /// will close the session when the lease is expired.
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the leasing procedure was executed successfully, false otherwise.
    bool send_keep_alive(ErrNo& error);

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join();

    /// @brief Triggers a single execution of join procedure: send the Join message
    /// @param error ``zenoh::ErrNo`` the error code
    /// @return true if the join procedure was executed successfully, false otherwise.
    bool send_join(ErrNo& error);
#endif

#ifdef __ZENOHCXX_ZENOHC
    /// @brief Concatenate the key expression and a string, resolving them if necessary
    /// @param k the key expression
    /// @param s ``std::string_view`` representing a key expression
    /// @return Newly allocated key expression ``zenoh::KeyExpr``
    /// @note zenoh-c only
    z::KeyExpr keyexpr_concat(const z::KeyExprView& k, const std::string_view& s);

    /// @brief Join two key expressions, inserting a separator between them
    /// @param a Key expression
    /// @param b Key expressio
    /// @return Newly allocated key expression ``zenoh::KeyExpr``
    /// @note zenoh-c only
    z::KeyExpr keyexpr_join(const z::KeyExprView& a, const z::KeyExprView& b);
#endif

    /// @brief Checks if the two key expressions are equal, resolving them if necessary
    /// @param a Key expression
    /// @param b Key expression
    /// @param error Error code returned by ``::z_keyexpr_equals`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expressions are equal
    bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

    /// @brief Checks if the two key expressions are equal, resolving them if necessary
    /// @param a Key expression
    /// @param b Key expression
    /// @return true the key expressions are equal
    bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b);

    /// @brief Checks if the key expression "a" includes expression "b", resolving them if necessary.
    // "Includes" means that the set defined by the key expression contains the set defined by the other key expression
    /// @param a Key expression
    /// @param b Key expression
    /// @param error Error code returned by ``::z_keyexpr_includes`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expression includes the other key expression
    bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

    /// @brief Checks if the key expression "a" includes key expression "b", resolving them if necessary.
    // "Includes" means that the set defined by the key expression contains the set defined by the other key expression
    /// @param a Key expression
    /// @param b Key expression
    /// @return true the key expression includes the other key expression
    bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b);

    /// @brief Checks if the key expression intersects with another key expression, resolving them if necessaRY.
    /// "Intersects" means that exists at least one key which is contained in both of the sets defined by the key
    /// expressions
    /// @param a Key expression
    /// @param b Key expression
    /// @param error Error code returned by ``::z_keyexpr_intersects`` (value < -1 if any of the key expressions is not
    /// valid)
    /// @return true the key expression intersects with the other key expression
    bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error);

    /// @brief Checks if the key expression intersects with another key expression, resolving them if necessaRY.
    /// "Intersects" means that exists at least one key which is contained in both of the sets defined by the key
    /// expressions
    /// @param a Key expression
    /// @param b Key expression
    /// @return true the key expression intersects with the other key expression
    bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b);

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

/// @brief Send data function returned by ``zenoh::reply_fifo_new`` and ``zenoh::reply_non_blocking_fifo_new``
/// @note zenoh-c only
class ClosureReplyChannelSend : public ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, z::Reply> {
   public:
    using ClosureMoveParam::ClosureMoveParam;
};

/// @brief Receive data function returned by ``zenoh::reply_fifo_new`` and ``zenoh::reply_non_blocking_fifo_new``
/// @note zenoh-c only
class ClosureReplyChannelRecv
    : public ClosureMoveParam<::z_owned_reply_channel_closure_t, ::z_owned_reply_t, z::Reply> {
   public:
    using ClosureMoveParam::ClosureMoveParam;
};

/// @brief Create a new blocking fifo channel
/// @param bound the maximum number of replies that can be stored in the channel. 0 means unbounded
/// @return a pair of closures: ``ClosureReplyChannelSend`` and ``ClosureReplyChannelRecv``
/// ``ClosureReplyChannelSend`` should be passed as callback to a ``Session::get`` call
/// ``ClosureReplyChannelRecv`` is a synchronous closure to be called from user code. It will block until either a
/// ``zenoh::Reply`` is available, which it will then return; or until the ``ClosureReplyChannelSend`` closure is
/// dropped and all replies have been consumed, at which point it will return an invalidated ``zenoh::Reply`` and so
/// will further calls.
/// @note zenoh-c only
inline std::pair<z::ClosureReplyChannelSend, z::ClosureReplyChannelRecv> reply_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

/// @brief Create a new non-blocking fifo channel
/// @param bound the maximum number of replies that can be stored in the channel
/// @return a pair of closures: ``ClosureReplyChannelSend`` and ``ClosureReplyChannelRecv``
/// ``ClosureReplyChannelSend`` should be passed as callback to a ``Session::get`` call
/// ``ClosureReplyChannelRecv`` is a synchronous closure to be called from user code. It will block until either a
/// ``zenoh::Reply`` is available, which it will then return; or until the ``ClosureReplyChannelSend`` closure is
/// dropped and all replies have been consumed, at which point it will return an invalidated ``zenoh::Reply`` and so
/// will further calls.
/// @note zenoh-c only
inline std::pair<z::ClosureReplyChannelSend, z::ClosureReplyChannelRecv> reply_non_blocking_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_non_blocking_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

#endif

#ifdef __DOXYGEN__
}  // end of namespace zenoh
#endif
