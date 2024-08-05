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

#include "../detail/closures.hxx"
#include "../detail/interop.hxx"
#include "base.hxx"
#include "closures.hxx"
#if (defined(SHARED_MEMORY) && defined(UNSTABLE))
#include "shm/buffer/buffer.hxx"
#endif

#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <functional>
#include <memory>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace zenoh {

namespace detail::closures {
extern "C" {
inline bool _zenoh_encode_iter(z_owned_bytes_t* b, void* context) {
    return IClosure<bool, z_owned_bytes_t*>::call_from_context(context, b);
}

inline void _zenoh_drop_with_context(void* data, void* context) {
    (void)data;
    IDroppable::delete_from_context(context);
}
}

}  // namespace detail::closures

class ZenohCodec;

/// @brief A Zenoh serialized data representation.
class Bytes : public Owned<::z_owned_bytes_t> {
   public:
    /// @name Constructors

    /// @brief Serializes data using default Zenoh codec.
    template <class T>
    Bytes(T data) : Bytes(Bytes::serialize(std::forward<T>(data))) {}

    /// @brief Construct a shallow copy of this data.
    Bytes clone() const {
        Bytes b;
        ::z_bytes_clone(&b._0, this->loan());
        return b;
    }

    /// @brief Construct an empty data.
    Bytes() : Owned(nullptr) { ::z_bytes_empty(detail::as_owned_c_ptr(*this)); }

    /// @name Methods

    /// @brief Get number of bytes in the pyload.
    size_t size() const { return ::z_bytes_len(this->loan()); }

    /// @brief Serialize specified type.
    ///
    /// @tparam T type to serialize
    /// @tparam Codec codec to use.
    /// @param data instance of T to serialize.
    /// @param codec instance of Codec to use.
    /// @return serialized data.
    template <class T, class Codec = ZenohCodec>
    static Bytes serialize(T&& data, Codec codec = Codec()) {
        return codec.serialize(std::forward<T>(data));
    }

    template <class ForwardIt, class Transform, class Codec = ZenohCodec>
    static Bytes serialize_from_iter_transformed(ForwardIt begin, ForwardIt end, Transform t, Codec codec = Codec()) {
        Bytes out;
        auto f = [current = begin, end, &codec, &t](z_owned_bytes_t* b) mutable {
            if (current == end) {
                ::z_null(b);
                return false;
            }
            auto it = current++;
            *b = Bytes::serialize(t(it), codec).take();
            return true;
        };
        using F = decltype(f);

        using ClosureType = typename detail::closures::Closure<F, closures::None, bool, z_owned_bytes_t*>;
        auto closure = ClosureType(std::forward<F>(f), closures::none);

        ::z_bytes_from_iter(detail::as_owned_c_ptr(out), detail::closures::_zenoh_encode_iter,
                                      closure.as_context());
        return out;
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
    template <class ForwardIt, class Codec = ZenohCodec>
    static Bytes serialize_from_iter(ForwardIt begin, ForwardIt end, Codec codec = Codec()) {
        return serialize_from_iter_transformed(
            begin, end, 
            [](ForwardIt it) -> typename ForwardIt::reference { return *it; },
            codec
        );
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T Type to deserialize into.
    /// @tparam Codec codec to use.
    /// @param res if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @param codec codec instance.
    /// @return deserialzied data.
    template <class T, class Codec = ZenohCodec>
    T deserialize(ZResult* err, Codec codec = Codec()) const {
        return codec.template deserialize<T>(*this, err);
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T type to deserialize into.
    /// @tparam Codec codec to use.
    /// @return deserialzied data.
    template <class T, class Codec = ZenohCodec>
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
        size_t read(uint8_t* dst, size_t len) { return ::z_bytes_reader_read(&this->_0, dst, len); }

        /// @brief Return the read position indicator.
        /// @return read position indicator on success or -1L if failure occurs.
        int64_t tell() { return ::z_bytes_reader_tell(&this->_0); }

        /// @brief Set the `reader` position indicator to the value pointed to by offset, starting from the current
        /// position.
        /// @param offset offset in bytes starting from the current position.
        /// @param res if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_current(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_CUR), err, "seek_from_current failed");
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset, starting from the start of the
        /// data.
        /// @param offset offset in bytes starting from the 0-th byte position.
        /// @param res if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_start(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_SET), err, "seek_from_start failed");
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset with respect to the end of the
        /// data.
        /// @param offset offset in bytes starting from end position.
        /// @param res if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_end(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_END), err, "seek_from_end failed");
        }
    };

    /// @brief Create data reader.
    /// @return reader instance.
    Reader reader() const { return Reader(::z_bytes_get_reader(this->loan())); }

    /// @brief A writer for Zenoh-serialized data.
    class Writer : public Owned<::z_owned_bytes_writer_t> {
       public:
        using Owned::Owned;

        /// @name Methods

        /// @brief Copy data from sepcified source into underlying ``Bytes`` instance.
        /// @param src source to copy data from.
        /// @param len number of bytes to copy from src to the underlying ``Bytes`` instance.
        /// @param res if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void write_all(const uint8_t* src, size_t len, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_write_all(this->loan(), src, len), err, "Failed to write data");
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
class Bytes::Iterator : Copyable<::z_bytes_iterator_t> {
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

inline Bytes::Iterator Bytes::iter() const { return Bytes::Iterator(::z_bytes_get_iterator(this->loan())); }

namespace detail {

template <class T>
struct ZenohDeserializer {};

template <>
struct ZenohDeserializer<std::string> {
    static std::string deserialize(const Bytes& b, ZResult* err = nullptr) {
        (void)err;
        auto reader = b.reader();
        std::string s(b.size(), '0');
        reader.read(reinterpret_cast<uint8_t*>(s.data()), s.size());
        return s;
    }
};

#if (defined(SHARED_MEMORY) && defined(UNSTABLE))
template <>
struct ZenohDeserializer<ZShm> {
    static ZShm deserialize(const Bytes& b, ZResult* err = nullptr) {
        ZShm shm(nullptr);
        __ZENOH_RESULT_CHECK(::z_bytes_deserialize_into_owned_shm(detail::loan(b), detail::as_owned_c_ptr(shm)), err,
                             "Failed to deserialize into ZShm!");
        return shm;
    }
};
#endif

template <class Allocator>
struct ZenohDeserializer<std::vector<uint8_t, Allocator>> {
    static std::vector<uint8_t, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        (void)err;
        auto reader = b.reader();
        std::vector<uint8_t, Allocator> v(b.size());
        reader.read(v.data(), b.size());
        return v;
    }
};

template <class A, class B>
struct ZenohDeserializer<std::pair<A, B>> {
    static std::pair<A, B> deserialize(const Bytes& b, ZResult* err = nullptr) {
        zenoh::Bytes ba, bb;
        __ZENOH_RESULT_CHECK(
            ::z_bytes_deserialize_into_pair(detail::loan(b), detail::as_owned_c_ptr(ba), detail::as_owned_c_ptr(bb)),
            err, "Failed to deserialize into std::pair");
        return {ZenohDeserializer<A>::deserialize(ba, err), ZenohDeserializer<B>::deserialize(bb, err)};
    }
};

template <class T, class Allocator>
struct ZenohDeserializer<std::vector<T, Allocator>> {
    static std::vector<T, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::vector<T, Allocator> v;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            v.push_back(bb->deserialize<T>(err));
        }

        return v;
    }
};

template <class T, class Allocator>
struct ZenohDeserializer<std::deque<T, Allocator>> {
    static std::deque<T, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::deque<T, Allocator> v;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            v.push_back(bb->deserialize<T>(err));
        }

        return v;
    }
};

template <class K, class H, class E, class Allocator>
struct ZenohDeserializer<std::unordered_set<K, H, E, Allocator>> {
    static std::unordered_set<K, H, E, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::unordered_set<K, H, E, Allocator> s;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            s.insert(bb->deserialize<K>(err));
        }

        return s;
    }
};

template <class K, class C, class Allocator>
struct ZenohDeserializer<std::set<K, C, Allocator>> {
    static std::set<K, C, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::set<K, C, Allocator> s;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            s.insert(bb->deserialize<K>(err));
        }

        return s;
    }
};

template <class K, class V, class H, class E, class Allocator>
struct ZenohDeserializer<std::unordered_map<K, V, H, E, Allocator>> {
    static std::unordered_map<K, V, H, E, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::unordered_map<K, V, H, E, Allocator> m;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            m.insert(bb->deserialize<std::pair<K, V>>(err));
        }

        return m;
    }
};

template <class K, class V, class C, class Allocator>
struct ZenohDeserializer<std::map<K, V, C, Allocator>> {
    static std::map<K, V, C, Allocator> deserialize(const Bytes& b, ZResult* err = nullptr) {
        std::map<K, V, C, Allocator> m;
        auto it = b.iter();
        for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
            m.insert(bb->deserialize<std::pair<K, V>>(err));
        }

        return m;
    }
};

#define __ZENOH_DESERIALIZE_ARITHMETIC(TYPE, EXT)                                            \
    template <>                                                                              \
    struct ZenohDeserializer<TYPE> {                                                         \
        static TYPE deserialize(const Bytes& b, ZResult* err = nullptr) {                    \
            TYPE t;                                                                          \
            __ZENOH_RESULT_CHECK(::z_bytes_deserialize_into_##EXT(detail::loan(b), &t), err, \
                                 "Failed to deserialize into " #TYPE);                       \
            return t;                                                                        \
        }                                                                                    \
    };

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
}  // namespace detail

struct Slice {
    const uint8_t* data;
    size_t len;
};

auto make_slice(const uint8_t* data, size_t len) {
    return Slice{data, len};
};

template<class Deleter>
struct OwnedSlice {
    uint8_t* data;
    size_t len;
    Deleter d;
};

template<class Deleter>
auto make_owned_slice(uint8_t* data, size_t len, Deleter&& d) {
    return OwnedSlice<std::remove_reference_t<Deleter>>{data, len, std::forward<Deleter>(d)};
};

class ZenohCodec {
    std::function<void(void* data)> _alias_guard;

    template<class G>
    ZenohCodec(G alias_guard) :_alias_guard(alias_guard) {}
public:
    ZenohCodec() :_alias_guard() {}

    /// @brief Serialize pointer and lenght by copying.
    Bytes serialize(const Slice& s) {
        Bytes b;
        if (!this->_alias_guard) {
            ::z_bytes_serialize_from_buf(detail::as_owned_c_ptr(b), s.data, s.len);
        } else {
            b = serialize(make_owned_slice(const_cast<uint8_t*>(s.data), s.len,  this->_alias_guard));
        }
        return b;
    }

    /// @brief Serialize pointer and length by moving.
    template<class Deleter> 
    Bytes serialize(OwnedSlice<Deleter>&& s) {
        Bytes b;
        uint8_t* data = s.data;
        size_t len = s.len;
        auto d = [s_in = std::move(s)]() mutable {
            s_in.d(s_in.data);
        };
        using D = decltype(d);
        using Dval = std::remove_reference_t<D>;
        using DroppableType = typename detail::closures::Droppable<Dval>;
        auto drop = DroppableType::into_context(std::forward<D>(d));
        ::z_bytes_from_buf(detail::as_owned_c_ptr(b), data, len, detail::closures::_zenoh_drop_with_context, drop);
        return b;
    }

    Bytes serialize(std::string_view s) {
        return serialize(Slice{reinterpret_cast<const uint8_t*>(s.data()), s.size()});
    }

    Bytes serialize(const char* s) { return serialize(std::string_view(s)); }

    Bytes serialize(const std::string& s) { return serialize(static_cast<std::string_view>(s)); }
    Bytes serialize(std::string&& s) {
        std::string *ptr = new std::string(std::move(s));
        auto deleter = [p = ptr](void *data) mutable {
            (void)data;
            delete p;
        };
        return serialize(make_owned_slice(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(ptr->c_str())), ptr->size(), std::move(deleter))
        ); 
    }

    Bytes serialize(Bytes&& b) { return std::move(b); }

    Bytes serialize(const Bytes& b) { return b.clone(); }

#if (defined(SHARED_MEMORY) && defined(UNSTABLE))
    Bytes serialize(ZShm&& shm, ZResult* err = nullptr) {
        Bytes b;
        __ZENOH_RESULT_CHECK(::z_bytes_serialize_from_shm(detail::as_owned_c_ptr(b), detail::as_owned_c_ptr(shm)), err,
                             "Failed to serialize ZShm");
        return b;
    }

    Bytes serialize(ZShmMut&& shm, ZResult* err = nullptr) {
        Bytes b;
        __ZENOH_RESULT_CHECK(::z_bytes_serialize_from_shm_mut(detail::as_owned_c_ptr(b), detail::as_owned_c_ptr(shm)),
                             err, "Failed to serialize ZShmMut");
        return b;
    }
#endif

    template <class Allocator>
    Bytes serialize(const std::vector<uint8_t, Allocator>& s) {
        auto b = ZenohCodec::serialize(make_slice(s.data(), s.size()));
        return b;
    }

    template <class Allocator>
    Bytes serialize(std::vector<uint8_t, Allocator>&& s) {
        std::vector<uint8_t, Allocator>* ptr = new std::vector<uint8_t, Allocator>(std::move(s));
        auto deleter = [p = ptr](void *data) mutable {
            (void)data;
            delete p;
        };
        return serialize(make_owned_slice(ptr->data(), ptr->size(), std::move(deleter))); 
    }

    template <class T, class Allocator>
    Bytes serialize(const std::vector<T, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class T, class Allocator>
    Bytes serialize(std::vector<T, Allocator>&& s) {
        auto b = Bytes::serialize_from_iter(std::make_move_iterator(s.begin()), std::make_move_iterator(s.end()), *this);
        s.clear();
        return b;
    }

    template <class T, class Allocator>
    Bytes serialize(const std::deque<T, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class T, class Allocator>
    Bytes serialize(std::deque<T, Allocator>&& s) {
        auto b = Bytes::serialize_from_iter(std::make_move_iterator(s.begin()), std::make_move_iterator(s.end()), *this);
        s.clear();
        return b;
    }

    template <class K, class H, class E, class Allocator>
    Bytes serialize(const std::unordered_set<K, H, E, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class H, class E, class Allocator>
    Bytes serialize(std::unordered_set<K, H, E, Allocator>&& s) {
        return Bytes::serialize_from_iter_transformed(
            s.begin(), s.end(),
            [&s](typename std::unordered_set<K, H, E, Allocator>::iterator it) -> K {
                return std::move(s.extract(it).value());
            },
            *this
        );
    }

    template <class K, class C, class Allocator>
    Bytes serialize(const std::set<K, C, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class C, class Allocator>
    Bytes serialize(std::set<K, C, Allocator>&& s) {
        return Bytes::serialize_from_iter_transformed(
            s.begin(), s.end(),
            [&s](typename std::set<K, C, Allocator>::iterator it) -> K {
                return std::move(s.extract(it).value());
            }
        );
    }

    template <class K, class V, class H, class E, class Allocator>
    Bytes serialize(const std::unordered_map<K, V, H, E, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class V, class H, class E, class Allocator>
    Bytes serialize(std::unordered_map<K, V, H, E, Allocator>&& s) {
        return Bytes::serialize_from_iter_transformed(
            s.begin(), s.end(),
            [&s](typename std::unordered_map<K, V, H, E, Allocator>::iterator it) {
                auto node = s.extract(it);
                return std::pair<K, V>(std::move(node.key()), std::move(node.mapped()));
            },
            *this
        );
    }

    

    template <class K, class V, class C, class Allocator>
    Bytes serialize(const std::map<K, V, C, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class V, class C, class Allocator>
    Bytes serialize(std::map<K, V, C, Allocator>&& s) {
        return Bytes::serialize_from_iter_transformed(
            s.begin(), s.end(),
            [&s](typename std::map<K, V, C, Allocator>::iterator it) {
                auto node = s.extract(it);
                return std::pair<K, V>(std::move(node.key()), std::move(node.mapped()));
            },
            *this
        );
    }

    template <class A, class B>
    Bytes serialize(const std::pair<A, B>& s) {
        auto ba = serialize(s.first);
        auto bb = serialize(s.second);
        Bytes b;
        ::z_bytes_from_pair(detail::as_owned_c_ptr(b), ::z_move(*detail::as_owned_c_ptr(ba)),
                                      ::z_move(*detail::as_owned_c_ptr(bb)));
        return b;
    }

    template <class A, class B>
    Bytes serialize(std::pair<A, B>&& s) {
        auto ba = serialize(std::move(s.first));
        auto bb = serialize(std::move(s.second));
        Bytes b;
        ::z_bytes_from_pair(detail::as_owned_c_ptr(b), ::z_move(*detail::as_owned_c_ptr(ba)),
                                      ::z_move(*detail::as_owned_c_ptr(bb)));
        return b;
    }

    template<class T>
    Bytes serialize(std::shared_ptr<T> s) {
        if (!this->_alias_guard) {
            T* ptr = s.get();
            ZenohCodec c(
                [p = std::move(s)](void* data) mutable { 
                    (void)data;
                    p = nullptr; 
                });
            return c.serialize(*ptr);
        } else {
            // pointer is inside higher level pointer so there is no need to modify the alias guard.
            return this->serialize(*s.get());
        }
    }

#define __ZENOH_SERIALIZE_ARITHMETIC(TYPE, EXT)                       \
    Bytes serialize(TYPE t) {                                  \
        Bytes b;                                                      \
        ::z_bytes_serialize_from_##EXT(detail::as_owned_c_ptr(b), t); \
        return b;                                                     \
    }

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

    template <class T>
    T deserialize(const Bytes& b, ZResult* err = nullptr) {
        return detail::ZenohDeserializer<T>::deserialize(b, err);
    }
};

}  // namespace zenoh
