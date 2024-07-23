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

#include "base.hxx"
#include "../detail/interop.hxx"
#include "../detail/closures.hxx"
#include "closures.hxx"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <deque>
#include <unordered_set>
#include <map>
#include <set>

namespace zenoh {

namespace detail::closures {
extern "C" {
    inline bool _zenoh_encode_iter(z_owned_bytes_t* b, void* context) {
        return IClosure<bool, z_owned_bytes_t*>::call_from_context(context, b);
    }
}

}

enum class ZenohCodecType {
    STANDARD,
    AVOID_COPY
};

template<ZenohCodecType ZT = ZenohCodecType::STANDARD>
struct ZenohCodec;

/// @brief A Zenoh serialized data representation.
class Bytes : public Owned<::z_owned_bytes_t> {
public:

    /// @name Constructors
    /// Serializes data using default Zenoh codec.
    template<class T>
    Bytes(T data) :Bytes(Bytes::serialize(std::forward<T>(data))) {}
    /// @brief Construct a shallow copy of this data.
    Bytes clone() const {
        Bytes b;
        ::z_bytes_clone(&b._0, this->loan());
        return b; 
    }

    /// @brief Construct an empty data.
    Bytes() 
        : Owned(nullptr) {
        ::z_bytes_empty(detail::as_owned_c_ptr(*this));
    }

    /// @name Methods

    /// @brief Get number of bytes in the pyload.
    size_t size() const {
        return ::z_bytes_len(this->loan());
    }

    /// @brief Serialize specified type.
    ///
    /// @tparam T type to serialize
    /// @tparam Codec codec to use.
    /// @param data instance of T to serialize.
    /// @param codec instance of Codec to use.
    /// @return serialized data.
    template<class T, class Codec = ZenohCodec<>>
    static Bytes serialize(const T& data, Codec codec = Codec()) {
        return codec.serialize(data);
    }

    /// @brief Serializes multiple pieces of data between begin and end iterators.
    ///
    /// The data can be later read using Bytes::Iterator provided by Bytes::iter() method.
    /// @tparam ForwardIt forward input iterator type
    /// @tparam Codec codec type.
    /// @param begin start of the iterator range.
    /// @param end end of the iterator range.
    /// @param codec codec instance.
    /// @return serialized data.
    template<class ForwardIt, class Codec = ZenohCodec<>> 
    static Bytes serialize_from_iter(ForwardIt begin, ForwardIt end, Codec codec = Codec()) {
        Bytes out;
        auto f = [current = begin, end, &codec] (z_owned_bytes_t* b) mutable {
            if (current == end) {
                ::z_null(b);
                return false;
            }
            *b = Bytes::serialize(*current, codec).take();
            current++;
            return true;
        };
        using F = decltype(f);

        using ClosureType = typename detail::closures::Closure<F, closures::None, bool, z_owned_bytes_t*>;
        auto closure = ClosureType(std::forward<F>(f), closures::none);
        
        ::z_bytes_serialize_from_iter(detail::as_owned_c_ptr(out), detail::closures::_zenoh_encode_iter, closure.as_context());
        return out;
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T Type to deserialize into.
    /// @tparam Codec codec to use.
    /// @param err if not null, the error code will be written to this location, otherwise ZException exception will be thrown in case of error.
    /// @param codec codec instance.
    /// @return deserialzied data.
    template<class T, class Codec = ZenohCodec<>>
    T deserialize(ZError* err, Codec codec = Codec()) const {
        return codec.template deserialize<T>(*this, err);
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T type to deserialize into.
    /// @tparam Codec codec to use.
    /// @return deserialzied data.
    template<class T, class Codec = ZenohCodec<>>
    T deserialize(Codec codec = Codec()) const {
        return codec.template deserialize<T>(*this, nullptr);
    }


    class Iterator;

    /// @brief Get iterator to multi-element data serialized previously using ``Bytes::serialize_from_iter``.
    /// @return iterator over multiple elements of data.
    Iterator iter() const;

    /// @brief A reader for Zenoh-serialized data.
    class Reader : public Copyable<::z_bytes_reader_t> {
    public:
        using Copyable::Copyable;

        /// @name Methods

        /// @brief Read data into specified destination.
        /// @param dst buffer where read data is written.
        /// @param len number of bytes to read.
        /// @return number of bytes that were read. Might be less than len if there is not enough data.
        size_t read(uint8_t* dst, size_t len) {
            return ::z_bytes_reader_read(&this->_0, dst, len);
        }

        /// @brief Return the read position indicator.
        /// @return read position indicator on success or -1L if failure occurs.
        int64_t tell() {
            return ::z_bytes_reader_tell(&this->_0);
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset, starting from the current position.
        /// @param offset offset in bytes starting from the current position.
        /// @param err if not null, the error code will be written to this location, otherwise ZException exception will be thrown in case of error.
        void seek_from_current(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(&this->_0, offset, SEEK_CUR),
                err,
                "seek_from_current failed"
            );
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset, starting from the start of the data.
        /// @param offset offset in bytes starting from the 0-th byte position.
        /// @param err if not null, the error code will be written to this location, otherwise ZException exception will be thrown in case of error.
        void seek_from_start(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(&this->_0, offset, SEEK_SET),
                err,
                "seek_from_start failed"
            );
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset with respect to the end of the data.
        /// @param offset offset in bytes starting from end position.
        /// @param err if not null, the error code will be written to this location, otherwise ZException exception will be thrown in case of error.
        void seek_from_end(int64_t offset, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_reader_seek(&this->_0, offset, SEEK_END),
                err,
                "seek_from_end failed"
            );
        }
    };

    /// @brief Create data reader.
    /// @return reader instance.
    Reader reader() const {
        return Reader(::z_bytes_get_reader(this->loan()));
    }

    /// @brief A writer for Zenoh-serialized data.
    class Writer : public Owned<::z_owned_bytes_writer_t> {
    public:
        using Owned::Owned;

        /// @name Methods

        /// @brief Copy data from sepcified source into underlying ``Bytes`` instance.
        /// @param src source to copy data from.
        /// @param len number of bytes to copy from src to the underlying ``Bytes`` instance.
        /// @param err if not null, the error code will be written to this location, otherwise ZException exception will be thrown in case of error.
        void write(const uint8_t* src, size_t len, ZError* err = nullptr) {
            __ZENOH_ERROR_CHECK(
                ::z_bytes_writer_write(this->loan(), src, len),
                err,
                "Failed to write data"
            );
        }
    };

    /// @brief Create data writer. 
    ///
    /// It is the user responsibility to ensure that there is at most one active writer at
    /// a given moment of time for a given ``Bytes`` instance.
    /// @return writer instance.
    Writer writer() {
        Writer w(nullptr);
        ::z_bytes_get_writer(this->loan(), detail::as_owned_c_ptr(w));
        return w;
    }
};

/// @brief An iterator over multi-element serialized data.
class Bytes::Iterator: Copyable<::z_bytes_iterator_t> {
public:
    using Copyable::Copyable;

    /// @name Methods

    /// @brief Return next element of serialized data.
    /// @return next element of serialized data, if the iterator reached the end, an empty optional will be returned.
    std::optional<Bytes> next() {
        std::optional<Bytes> b(std::in_place);
        if (!::z_bytes_iterator_next(&this->_0, detail::as_owned_c_ptr(b.value()))) {
            b.reset();
        }
        return b;
    }


};

inline Bytes::Iterator Bytes::iter() const {
    return Bytes::Iterator(::z_bytes_get_iterator(this->loan()));
}




namespace detail {

template<class T> 
struct ZenohDeserializer {};

template<>
struct ZenohDeserializer<std::string> {
    static std::string deserialize(const Bytes& b, ZError* err = nullptr) {
        (void)err;
        auto reader = b.reader();
        std::string s(b.size(), '0');
        reader.read(reinterpret_cast<uint8_t*>(s.data()), s.size());
        return s;
    }
};

template<class Allocator>
struct ZenohDeserializer<std::vector<uint8_t, Allocator>> {
    static std::vector<uint8_t, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        (void)err;
        auto reader = b.reader();
        std::vector<uint8_t, Allocator> v(b.size());
        reader.read(v.data(), b.size());
        return v;
    }
};

template<class A, class B>
struct ZenohDeserializer<std::pair<A, B>> {
    static std::pair<A, B> deserialize(const Bytes& b, ZError* err = nullptr) {
        zenoh::Bytes ba, bb;
        __ZENOH_ERROR_CHECK(
            ::z_bytes_deserialize_into_pair(detail::loan(b), detail::as_owned_c_ptr(ba), detail::as_owned_c_ptr(bb)),
            err,
            "Failed to deserialize into std::pair"
        );
        return {ZenohDeserializer<A>::deserialize(ba, err), ZenohDeserializer<B>::deserialize(bb, err)};
    }
};

template<class T, class Allocator>
struct ZenohDeserializer<std::vector<T, Allocator>> {
    static std::vector<T, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::vector<T, Allocator> v;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            v.push_back(bb->deserialize<T>(err));
        }
        
        return v;
    }
};

template<class T, class Allocator>
struct ZenohDeserializer<std::deque<T, Allocator>> {
    static std::deque<T, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::deque<T, Allocator> v;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            v.push_back(bb->deserialize<T>(err));
        }
        
        return v;
    }
};

template<class K, class H, class E, class Allocator>
struct ZenohDeserializer<std::unordered_set<K, H, E, Allocator>> {
    static std::unordered_set<K, H, E, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::unordered_set<K, H, E, Allocator> s;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            s.insert(bb->deserialize<K>(err));
        }
        
        return s;
    }
};

template<class K, class C,  class Allocator>
struct ZenohDeserializer<std::set<K, C, Allocator>> {
    static std::set<K, C, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::set<K, C, Allocator> s;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            s.insert(bb->deserialize<K>(err));
        }
        
        return s;
    }
};

template<class K, class V, class H, class E, class Allocator>
struct ZenohDeserializer<std::unordered_map<K, V, H, E, Allocator>> {
    static std::unordered_map<K, V, H, E, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::unordered_map<K, V, H, E, Allocator> m;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            m.insert(bb->deserialize<std::pair<K, V>>(err));
        }
        
        return m;
    }
};

template<class K, class V, class C, class Allocator>
struct ZenohDeserializer<std::map<K, V, C, Allocator>> {
    static std::map<K, V, C, Allocator> deserialize(const Bytes& b, ZError* err = nullptr) {
        std::map<K, V, C, Allocator> m;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            m.insert(bb->deserialize<std::pair<K, V>>(err));
        }
        
        return m;
    }
};

#define __ZENOH_DESERIALIZE_ARITHMETIC(TYPE, EXT) \
template<> \
struct ZenohDeserializer<TYPE> { \
    static TYPE deserialize(const Bytes& b, ZError* err = nullptr) { \
        TYPE t;\
        __ZENOH_ERROR_CHECK( \
            ::z_bytes_deserialize_into_##EXT(detail::loan(b), &t), \
            err, \
            "Failed to deserialize into "#TYPE \
        ); \
        return t; \
    } \
}; \

__ZENOH_DESERIALIZE_ARITHMETIC(uint8_t, uint8);
__ZENOH_DESERIALIZE_ARITHMETIC(uint16_t, uint16);
__ZENOH_DESERIALIZE_ARITHMETIC(uint32_t, uint32);
__ZENOH_DESERIALIZE_ARITHMETIC(uint64_t, uint64);

__ZENOH_DESERIALIZE_ARITHMETIC(int8_t, int8);
__ZENOH_DESERIALIZE_ARITHMETIC(int16_t, int16);
__ZENOH_DESERIALIZE_ARITHMETIC(int32_t, int32);
__ZENOH_DESERIALIZE_ARITHMETIC(int64_t, int64);

__ZENOH_DESERIALIZE_ARITHMETIC(float, float);
__ZENOH_DESERIALIZE_ARITHMETIC(double, double);

#undef __ZENOH_DESERIALIZE_ARITHMETIC
}

template<ZenohCodecType ZT>
struct ZenohCodec {
    static Bytes serialize(std::string_view s) {
        return ZenohCodec::serialize(std::make_pair(reinterpret_cast<const uint8_t*>(s.data()), s.size()));
    }

    static Bytes serialize(const char* s) {
        return ZenohCodec::serialize(std::string_view(s));
    }

    static Bytes serialize(const std::string& s) {
        return ZenohCodec::serialize(static_cast<std::string_view>(s));
    }

    static Bytes serialize(Bytes&& b) {
        return std::move(b);
    }

    static Bytes serialize(const Bytes& b) {
        return b.clone();
    }

    static Bytes serialize(const std::pair<const uint8_t*, size_t>& s) {
        Bytes b;
        if constexpr (ZT == ZenohCodecType::AVOID_COPY) {
            ::z_bytes_serialize_from_slice(detail::as_owned_c_ptr(b), s.first, s.second);
        } else {
            ::z_bytes_serialize_from_slice_copy(detail::as_owned_c_ptr(b), s.first, s.second);
        }
        return b;
    }

    static Bytes serialize(const std::pair<uint8_t*, size_t>& s) {
        return serialize(std::pair<const uint8_t*, size_t>(s.first, s.second));
    }

    template<class Allocator>
    static Bytes serialize(const std::vector<uint8_t, Allocator>& s) {
        return ZenohCodec::serialize(std::make_pair<const uint8_t*, size_t>(s.data(), s.size()));
    }

    template<class T, class Allocator>
    static Bytes serialize(const std::vector<T, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class T, class Allocator>
    static Bytes serialize(const std::deque<T, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class K, class H, class E, class Allocator>
    static Bytes serialize(const std::unordered_set<K, H, E, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class K, class C, class Allocator>
    static Bytes serialize(const std::set<K, C, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class K, class V, class H, class E, class Allocator>
    static Bytes serialize(const std::unordered_map<K, V, H, E, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class K, class V, class C, class Allocator>
    static Bytes serialize(const std::map<K, V, C, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end());
    }

    template<class A, class B>
    static Bytes serialize(const std::pair<A, B>& s) {
        auto ba = ZenohCodec::serialize(s.first);
        auto bb = ZenohCodec::serialize(s.second);
        Bytes b;
        ::z_bytes_serialize_from_pair(detail::as_owned_c_ptr(b), ::z_move(*detail::as_owned_c_ptr(ba)), ::z_move(*detail::as_owned_c_ptr(bb)));
        return b;
    }

#define __ZENOH_SERIALIZE_ARITHMETIC(TYPE, EXT) \
    static Bytes serialize(TYPE t) { \
        Bytes b; \
        ::z_bytes_serialize_from_##EXT(detail::as_owned_c_ptr(b), t); \
        return b; \
    } \

    __ZENOH_SERIALIZE_ARITHMETIC(uint8_t, uint8);
    __ZENOH_SERIALIZE_ARITHMETIC(uint16_t, uint16);
    __ZENOH_SERIALIZE_ARITHMETIC(uint32_t, uint32);
    __ZENOH_SERIALIZE_ARITHMETIC(uint64_t, uint64);

    __ZENOH_SERIALIZE_ARITHMETIC(int8_t, int8);
    __ZENOH_SERIALIZE_ARITHMETIC(int16_t, int16);
    __ZENOH_SERIALIZE_ARITHMETIC(int32_t, int32);
    __ZENOH_SERIALIZE_ARITHMETIC(int64_t, int64);

    __ZENOH_SERIALIZE_ARITHMETIC(float, float);
    __ZENOH_SERIALIZE_ARITHMETIC(double, double);
#undef __ZENOH_SERIALIZE_ARITHMETIC

    template<class T>
    static T deserialize(const Bytes& b, ZError* err = nullptr) {
        return detail::ZenohDeserializer<T>::deserialize(b, err);
    }
};



}

