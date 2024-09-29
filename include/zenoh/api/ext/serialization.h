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

#include "../bytes.hxx"
#include "../base.hxx"
#include "../interop.hxx"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <tuple>
#include <iterator>

namespace zenoh {
namespace ext {

#if defined(Z_FEATURE_UNSTABLE_API)

class Serializer;
class Deserializer;

namespace detail {
#define __ZENOH_SERIALIZE_ARITHMETIC(TYPE, EXT)                        \
void serialize_with_serializer(zenoh::ext::Serializer& serializer, TYPE t) {                                             \
    ::ze_serializer_serialize_##EXT(interop::as_loaned_c_ptr(serializer), t);                                               \
}

__ZENOH_SERIALIZE_ARITHMETIC(uint8_t, uint8)
__ZENOH_SERIALIZE_ARITHMETIC(uint16_t, uint16)
__ZENOH_SERIALIZE_ARITHMETIC(uint32_t, uint32)
__ZENOH_SERIALIZE_ARITHMETIC(uint64_t, uint64)
__ZENOH_SERIALIZE_ARITHMETIC(int8_t, int8)
__ZENOH_SERIALIZE_ARITHMETIC(int16_t, int16)
__ZENOH_SERIALIZE_ARITHMETIC(int32_t, int32)
__ZENOH_SERIALIZE_ARITHMETIC(int64_t, int64)
__ZENOH_SERIALIZE_ARITHMETIC(float, float)
__ZENOH_SERIALIZE_ARITHMETIC(double, double)

#undef __ZENOH_SERIALIZE_ARITHMETIC
void serialize_with_serializer(zenoh::ext::Serializer& serializer, std::string_view value) {
    ::z_view_string_t s;
    z_view_string_from_substr(&s, value.data(), value.size());
    ::ze_serializer_serialize_string(interop::as_loaned_c_ptr(serializer), ::z_loan(s));
}

void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::string& value) {
    return serialize_with_serializer(serializer, std::string_view(value));
}

void serialize_with_serializer(zenoh::ext::Serializer& serializer, const char* value) {
    return serialize_with_serializer(serializer, std::string_view(value));
}

template<class ...Types>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::tuple<Types...>& value) {
    std::apply([&serializer](const auto&... v) { (serialize_with_serializer(serializer, v), ...); } );
}

template<class X, class Y>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::pair<X, Y>& value) {
    serialize_with_serializer(value.first);
    serialize_with_serializer(value.second);
}

template<class It>
void _serialize_sequence_with_serializer(zenoh::ext::Serializer& serializer, It begin, It end, size_t n) {
    ::ze_serializer_serialize_sequence_begin(zenoh::interop::as_loaned_c_ptr(serializer) ,n);
    for (const auto it = begin; it != end; ++it) {
        serialize_with_serializer(serializer, *it);
    }
    ::ze_serializer_serialize_sequence_end(zenoh::interop::as_loaned_c_ptr(serializer));
}

template<class T, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::vector<T>& value) {
    _serialize_sequence_with_serializer(v.size(), v.begin(), b.end());
}

template <class T, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::deque<T, Allocator>& value) {
    _serialize_sequence_with_serializer(value.size(), value.begin(), value.end());
}

template <class K, class H, class E, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::unordered_set<K, H, E, Allocator>& value) {
    return _serialize_sequence_with_serializer(serializer, values.size(), value.begin(), value.end());
}

template <class K, class Compare, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::set<K, Compare, Allocator>& value) {
    return _serialize_sequence_with_serializer(serializer, values.size(), value.begin(), value.end());
}

template <class K, class V, class H, class E, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::unordered_map<K, V, H, E, Allocator>& value) {
    return _serialize_sequence_with_serializer(serializer, values.size(), value.begin(), value.end());
}

template <class K, class V, class Compare, class Allocator>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::map<K, V, Compare, Allocator>& value) {
    return _serialize_sequence_with_serializer(serializer, values.size(), value.begin(), value.end());
}


#define __ZENOH_DESERIALIZE_ARITHMETIC(TYPE, EXT)                        \
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, TYPE& t, zenoh::ZResult* err = nullptr) { \
    __ZENOH_RESULT_CHECK( \
        ::ze_deserializer_deserialize_##EXT(interop::as_copyable_c_ptr(deserializer), &t),              \
        err, \
        "Deserialization failure"                                 \
    ); \
    return err == nullptr || *err == Z_OK; \
}
\

__ZENOH_DESERIALIZE_ARITHMETIC(uint8_t, uint8)
__ZENOH_DESERIALIZE_ARITHMETIC(uint16_t, uint16)
__ZENOH_DESERIALIZE_ARITHMETIC(uint32_t, uint32)
__ZENOH_DESERIALIZE_ARITHMETIC(uint64_t, uint64)
__ZENOH_DESERIALIZE_ARITHMETIC(int8_t, int8)
__ZENOH_DESERIALIZE_ARITHMETIC(int16_t, int16)
__ZENOH_DESERIALIZE_ARITHMETIC(int32_t, int32)
__ZENOH_DESERIALIZE_ARITHMETIC(int64_t, int64)
__ZENOH_DESERIALIZE_ARITHMETIC(float, float)
__ZENOH_DESERIALIZE_ARITHMETIC(double, double)

#undef __ZENOH_DESERIALIZE_ARITHMETIC
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::string& value, zenoh::ZResult* err = nullptr) {
    z_owned_string_t s;
    __ZENOH_RESULT_CHECK(
        ::ze_deserializer_deserialize_string(interop::as_copyable_c_ptr(deserializer), &s),             
        err,
        "Deserialization failure"                                
    );
    value = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
    return err == nullptr || *err == Z_OK;
}

template<class ...Types>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::tuple<Types...>& t, zenoh::ZResult* err = nullptr)  {
    return std::apply([&deserializer, err](const auto&... v) {
            bool res = true;
            res = deserialize_with_deserializer(deserializer, v, err) && ...; 
            return res;
        }
    );
}

template<class X, class Y>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::pair<X, Y>& value, zenoh::ZResult* err = nullptr) {
    return deserialize_with_deserializer(deserializer, value.first, err)
        && deserialize_with_deserializer(deserialzier, value.second, err);
}

#define _ZENOH_DESERIALIZE_SEQUENCE_BEGIN \
size_t len; \
__ZENOH_RESULT_CHECK( \
    ::ze_deserializer_deserialize_sequence_begin(zenoh::interop::as_copyable_c_ptr(deserializer), &len), \
    err, \
    "Deserialization failure:: Failed to read sequence length" \
); \
if (err != nullptr && *err != Z_OK) return false;

#define _ZENOH_DESERIALIZE_SEQUENCE_END \
__ZENOH_RESULT_CHECK( \
    ::ze_serializer_serialize_sequence_end(), \
    err, \
    "Deserialization failure:: Failed to finalize sequence read" \
); \
return (err == nullptr || *err == Z_OK);

template<class T, class Allocator>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::vector<T>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    value.reserve(value.size() + len);
    for (size_t i = 0; i < len; ++i) {
        T v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.push_back(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class T, class Allocator>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::deque<T, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        T v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.push_back(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class H, class E, class Allocator>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::unordered_set<K, H, E, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        K v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class Compare, class Allocator>
void deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::set<K, Compare, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        K v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class V, class H, class E, class Allocator>
void deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::unordered_map<K, V, H, E, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        std::pair<K, V> v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class V, class Compare, class Allocator>
void deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::map<K, V, Compare, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        std::pair<K, V> v;
        if (!deserialize_with_deserializer(deserializer, &v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

#undef _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
#undef _ZENOH_DESERIALIZE_SEQUENCE_END

}

/// @brief A Zenoh data serializer used for incremental serialization of several values.
/// I.e. data produced by subsequent calls to `Serializer::serialize` can be read by corresponding calls to
/// `Deserializer::deserialize` in the same order (or alternatively by a single call to `z_deserialize`
/// into tuple of serialized types).
class Serializer : public Owned<::ze_owned_serializer_t> {
public:
    /// @name Constructors

    /// Constructs an empty writer.
    Serializer() :Owned(nullptr) {
        ::ze_serializer_empty(interop::as_owned_c_ptr(*this));
    }

    /// @name Methods
    /// @brief Serialize specified value.
    template<class T>
    void serialize(const T& value) {
        return detail::serialize_with_serializer(*this, value);
    }


    /// @brief Finalize serialization and return underlying `Bytes` object.
    /// @return Underlying `Bytes` object.
    Bytes finish() && {
        Bytes b;
        ::ze_serializer_finish(interop::as_moved_c_ptr(*this), interop::as_owned_c_ptr(b));
        return b;
    }
};

/// @brief A Zenoh data deserializer used for incremental deserialization of several values.
/// I.e. data produced by subsequent calls to `Serializer::serialize` can be read by corresponding calls to
/// `Deserializer::deserialize` in the same order (or alternatively by a single call to `z_deserialize`
/// into tuple of serialized types).
class Deserializer : public Copyable<::ze_deserializer_t> {
public:
    /// @name Constructors

    /// @brief Construct deserializer for the specified data.
    /// @param b Data to initialize deserializer with. 
    Deserializer(const Bytes& b) :Copyable(::ze_deserializer_from_bytes(zenoh::interop::as_loaned_c_ptr(b))) {}

    /// @name Methods

    /// @brief Deserialize into value of specified type.
    template<class T>
    T deserialize(zenoh::ZResult* err = nullptr) {
        T t;
        detail::deserialize_with_deserializer(*this, &t, err);
        return t;
    }
};

/// @brief Serializes a single value and returns corresponding `Bytes`.
template<class T>
zenoh::Bytes serialize(const T& value) {
    Serializer s;
    s.serialize(value);
    return std::move(s).finish();
}



/// @brief Serializes a single value and returns corresponding `Bytes`.
template<class T>
T deserialize(const zenoh::Bytes& bytes, zenoh::ZResult* err = nullptr) {
    Deserializer d(bytes);
    // TODO return error if not all bytes are used.
    return d.deserialize(err);
}

#endif
}
}