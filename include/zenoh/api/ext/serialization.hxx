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

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../base.hxx"
#include "../bytes.hxx"
#include "../interop.hxx"

namespace zenoh {
namespace ext {

/// @brief A Zenoh data serializer used for incremental serialization of several values.
/// I.e. data produced by subsequent calls to `Serializer::serialize` can be read by corresponding calls to
/// `Deserializer::deserialize` in the same order (or alternatively by a single call to `z_deserialize`
/// into tuple of serialized types).
class Serializer : public Owned<::ze_owned_serializer_t> {
   public:
    /// @name Constructors

    /// Constructs an empty serializer.
    Serializer() : Owned(nullptr) { ::ze_serializer_empty(interop::as_owned_c_ptr(*this)); }

    /// @name Methods

    /// @brief Serialize specified value and append it to the underlying `Bytes`.
    /// @param value value to serialize.
    template <class T>
    void serialize(const T& value);

    /// @brief Finalize serialization and return the underlying `Bytes` object.
    /// @return underlying `Bytes` object.
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
    /// @param b data to initialize deserializer with.
    Deserializer(const Bytes& b) : Copyable(::ze_deserializer_from_bytes(zenoh::interop::as_loaned_c_ptr(b))) {}

    /// @name Methods

    /// @brief Deserialize next portion of data into specified type.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception
    /// will be thrown in case of error.
    /// @return deserialized value.
    template <class T>
    T deserialize(zenoh::ZResult* err = nullptr);

    /// @brief Checks if deserializer has parsed all the data.
    /// @return ``true`` if there is no more data to parse, ``false`` otherwise.
    bool is_done() const { return ::ze_deserializer_is_done(&this->_0); }
};

/// @brief Serialize a single value into `Bytes`.
/// @param value value to serialize.
/// @return 'Bytes' containing serialized value.
template <class T>
zenoh::Bytes serialize(const T& value) {
    Serializer s;
    s.serialize(value);
    return std::move(s).finish();
}

/// @brief Deserialize `Bytes` corresponding to a single serialized value.
/// @param bytes data to deserialize.
/// @param err if not null, the result code will be written to this location, otherwise ZException exception
/// will be thrown in case of error.
/// @return deserialized value.
template <class T>
T deserialize(const zenoh::Bytes& bytes, zenoh::ZResult* err = nullptr) {
    Deserializer d(bytes);
    T t = d.deserialize<T>(err);
    if (!d.is_done() && (err == nullptr || *err == Z_OK)) {
        __ZENOH_RESULT_CHECK(Z_EDESERIALIZE, err, "Payload contains more bytes than required for deserialization");
    }
    return t;
}

namespace detail {
template <class T>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const T& t);
template <class T>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, T& t, ZResult* err = nullptr);

#define __ZENOH_SERIALIZE_ARITHMETIC(TYPE, EXT)                                                 \
    inline void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, TYPE t) { \
        ::ze_serializer_serialize_##EXT(zenoh::interop::as_loaned_c_ptr(serializer), t);        \
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
__ZENOH_SERIALIZE_ARITHMETIC(bool, bool)

#undef __ZENOH_SERIALIZE_ARITHMETIC
inline void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, std::string_view value) {
    ::z_view_string_t s;
    z_view_string_from_substr(&s, value.data(), value.size());
    ::ze_serializer_serialize_string(interop::as_loaned_c_ptr(serializer), ::z_loan(s));
}

inline void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::string& value) {
    return __zenoh_serialize_with_serializer(serializer, std::string_view(value));
}

inline void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const char* value) {
    return __zenoh_serialize_with_serializer(serializer, std::string_view(value));
}

template <class... Types>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::tuple<Types...>& value) {
    std::apply([&serializer](const auto&... v) { (serialize_with_serializer(serializer, v), ...); }, value);
}

template <class X, class Y>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::pair<X, Y>& value) {
    serialize_with_serializer(serializer, value.first);
    serialize_with_serializer(serializer, value.second);
}

template <class It>
void __serialize_sequence_with_serializer(zenoh::ext::Serializer& serializer, It begin, It end, size_t n) {
    ::ze_serializer_serialize_sequence_length(zenoh::interop::as_loaned_c_ptr(serializer), n);
    for (auto it = begin; it != end; ++it) {
        serialize_with_serializer(serializer, *it);
    }
}

template <class T, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::vector<T, Allocator>& value) {
    __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class T, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::deque<T, Allocator>& value) {
    __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class K, class H, class E, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer,
                                       const std::unordered_set<K, H, E, Allocator>& value) {
    return __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class K, class Compare, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer,
                                       const std::set<K, Compare, Allocator>& value) {
    return __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class K, class V, class H, class E, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer,
                                       const std::unordered_map<K, V, H, E, Allocator>& value) {
    return __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class K, class V, class Compare, class Allocator>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer,
                                       const std::map<K, V, Compare, Allocator>& value) {
    return __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class T, size_t N>
void __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const std::array<T, N>& value) {
    return __serialize_sequence_with_serializer(serializer, value.begin(), value.end(), value.size());
}

template <class T>
void serialize_with_serializer(zenoh::ext::Serializer& serializer, const T& t) {
    __zenoh_serialize_with_serializer(serializer, t);
}

#define __ZENOH_DESERIALIZE_ARITHMETIC(TYPE, EXT)                                                                    \
    inline bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, TYPE& t,               \
                                                      zenoh::ZResult* err = nullptr) {                               \
        __ZENOH_RESULT_CHECK(::ze_deserializer_deserialize_##EXT(interop::as_copyable_c_ptr(deserializer), &t), err, \
                             "Deserialization failure");                                                             \
        return err == nullptr || *err == Z_OK;                                                                       \
    }

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
__ZENOH_DESERIALIZE_ARITHMETIC(bool, bool)

#undef __ZENOH_DESERIALIZE_ARITHMETIC
inline bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::string& value,
                                                  zenoh::ZResult* err = nullptr) {
    z_owned_string_t s;
    __ZENOH_RESULT_CHECK(::ze_deserializer_deserialize_string(interop::as_copyable_c_ptr(deserializer), &s), err,
                         "Deserialization failure");
    value = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
    return err == nullptr || *err == Z_OK;
}

template <class... Types>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::tuple<Types...>& t,
                                           zenoh::ZResult* err = nullptr) {
    return std::apply(
        [&deserializer, err](auto&... v) {
            bool res = true;
            res = res && (deserialize_with_deserializer(deserializer, v, err) && ...);
            return res;
        },
        t);
}

template <class X, class Y>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::pair<X, Y>& value,
                                           zenoh::ZResult* err = nullptr) {
    return deserialize_with_deserializer(deserializer, value.first, err) &&
           deserialize_with_deserializer(deserializer, value.second, err);
}

#define _ZENOH_DESERIALIZE_SEQUENCE_BEGIN                                                                          \
    size_t len;                                                                                                    \
    __ZENOH_RESULT_CHECK(                                                                                          \
        ::ze_deserializer_deserialize_sequence_length(zenoh::interop::as_copyable_c_ptr(deserializer), &len), err, \
        "Deserialization failure:: Failed to read sequence length");                                               \
    if (err != nullptr && *err != Z_OK) return false;

#define _ZENOH_DESERIALIZE_SEQUENCE_END return (err == nullptr || *err == Z_OK);

template <class T, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::vector<T, Allocator>& value,
                                           zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    value.reserve(value.size() + len);
    for (size_t i = 0; i < len; ++i) {
        T v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.push_back(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class T, size_t N>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::array<T, N>& value,
                                           zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    if (len != N && (err == nullptr || *err == Z_OK)) {
        __ZENOH_RESULT_CHECK(Z_EDESERIALIZE, err, "Incorrect sequence size");
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!deserialize_with_deserializer(deserializer, value[i], err)) return false;
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class T, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, std::deque<T, Allocator>& value,
                                           zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        T v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.push_back(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class H, class E, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer,
                                           std::unordered_set<K, H, E, Allocator>& value,
                                           zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        K v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class Compare, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer,
                                           std::set<K, Compare, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        K v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class V, class H, class E, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer,
                                           std::unordered_map<K, V, H, E, Allocator>& value,
                                           zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        std::pair<K, V> v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

template <class K, class V, class Compare, class Allocator>
bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer,
                                           std::map<K, V, Compare, Allocator>& value, zenoh::ZResult* err = nullptr) {
    _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
    for (size_t i = 0; i < len; ++i) {
        std::pair<K, V> v;
        if (!deserialize_with_deserializer(deserializer, v, err)) return false;
        value.insert(std::move(v));
    }
    _ZENOH_DESERIALIZE_SEQUENCE_END
}

#undef _ZENOH_DESERIALIZE_SEQUENCE_BEGIN
#undef _ZENOH_DESERIALIZE_SEQUENCE_END

template <class T>
bool deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, T& t, ZResult* err) {
    return __zenoh_deserialize_with_deserializer(deserializer, t, err);
}

}  // namespace detail

template <class T>
void Serializer::serialize(const T& value) {
    return detail::serialize_with_serializer(*this, value);
}

template <class T>
T Deserializer::deserialize(zenoh::ZResult* err) {
    T t;
    detail::deserialize_with_deserializer(*this, t, err);
    return t;
}

}  // namespace ext
}  // namespace zenoh
