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
#include "../detail/commons.hxx"
#include "base.hxx"
#include "closures.hxx"
#include "interop.hxx"
#if (defined(SHARED_MEMORY) && defined(UNSTABLE))
#include "shm/buffer/buffer.hxx"
#endif

#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <map>
#include <memory>
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
        ::z_bytes_clone(&b._0, interop::as_loaned_c_ptr(*this));
        return b;
    }

    /// @brief Construct an empty data.
    Bytes() : Owned(nullptr) { ::z_bytes_empty(interop::as_owned_c_ptr(*this)); }

    /// @name Methods

    /// @brief Get number of bytes in the pyload.
    size_t size() const { return ::z_bytes_len(interop::as_loaned_c_ptr(*this)); }

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
        Bytes out;
        auto f = [current = begin, end, &codec](z_owned_bytes_t* b) mutable {
            if (current == end) {
                ::z_internal_null(b);
                return false;
            }
            // increment current, in case iterator dereference might invalidate it, which happens
            // for map/set elements extraction while serializing std::move'd map/set
            auto it = current++;
            *b = interop::move_to_c_obj(Bytes::serialize(*it, codec));
            return true;
        };
        using F = decltype(f);

        using ClosureType = typename detail::closures::Closure<F, closures::None, bool, z_owned_bytes_t*>;
        auto closure = ClosureType(std::forward<F>(f), closures::none);

        ::z_bytes_from_iter(interop::as_owned_c_ptr(out), detail::closures::_zenoh_encode_iter, closure.as_context());
        return out;
    }

    /// @brief Deserialize into specified type.
    ///
    /// @tparam T Type to deserialize into.
    /// @tparam Codec codec to use.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
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
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_current(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_CUR), err, "seek_from_current failed");
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset, starting from the start of the
        /// data.
        /// @param offset offset in bytes starting from the 0-th byte position.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_start(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_SET), err, "seek_from_start failed");
        }

        /// @brief Set the `reader` position indicator to the value pointed to by offset with respect to the end of the
        /// data.
        /// @param offset offset in bytes starting from end position.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_end(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_END), err, "seek_from_end failed");
        }

        /// @brief Read bounded data previously written by ``Bytes::Writer::write_bounded``.
        ///
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        Bytes read_bounded(ZResult* err = nullptr) {
            Bytes b;
            __ZENOH_RESULT_CHECK(::z_bytes_reader_read_bounded(&this->_0, &b._0), err,
                                 "Failed to append data with boundaries");
            return b;
        }
    };

    /// @brief Create data reader.
    /// @return reader instance.
    Reader reader() const { return Reader(::z_bytes_get_reader(interop::as_loaned_c_ptr(*this))); }

    /// @brief A writer for Zenoh-serialized data.
    class Writer : public Copyable<::z_bytes_writer_t> {
       public:
        using Copyable::Copyable;

        /// @name Methods

        /// @brief Copy data from sepcified source into underlying ``Bytes`` instance.
        /// @param src source to copy data from.
        /// @param len number of bytes to copy from src to the underlying ``Bytes`` instance.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void write_all(const uint8_t* src, size_t len, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_write_all(&this->_0, src, len), err, "Failed to write data");
        }

        /// @brief Appends another `Bytes` instance.
        /// This allows to compose a serialized data out of multiple `Bytes` that may point to different memory regions.
        /// Said in other terms, it allows to create a linear view on different memory regions without copy.
        ///
        /// @param data data to append.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void append(Bytes&& data, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_append(&this->_0, z_move(data._0)), err, "Failed to append data");
        }

        /// @brief Append another `Bytes` instance, with boundaries information. It would allow to read the same piece
        /// of data using ``Bytes::reader::read_bounded``.
        ///
        /// @param data data to append.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void append_bounded(Bytes&& data, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_append_bounded(&this->_0, z_move(data._0)), err,
                                 "Failed to append data with boundaries");
        }
    };

    /// @brief Create data writer.
    ///
    /// It is the user responsibility to ensure that there is at most one active writer at
    /// a given moment of time for a given ``Bytes`` instance.
    /// @return writer instance.
    Writer writer() { return Writer(::z_bytes_get_writer(interop::as_loaned_c_ptr(*this))); }
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
        if (!::z_bytes_iterator_next(&this->_0, interop::as_owned_c_ptr(b.value()))) {
            b.reset();
        }
        return b;
    }
};

inline Bytes::Iterator Bytes::iter() const {
    return Bytes::Iterator(::z_bytes_get_iterator(interop::as_loaned_c_ptr(*this)));
}

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
        __ZENOH_RESULT_CHECK(
            ::z_bytes_deserialize_into_owned_shm(interop::as_loaned_c_ptr(b), interop::as_owned_c_ptr(shm)), err,
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
        __ZENOH_RESULT_CHECK(::z_bytes_deserialize_into_pair(interop::as_loaned_c_ptr(b), interop::as_owned_c_ptr(ba),
                                                             interop::as_owned_c_ptr(bb)),
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

#define __ZENOH_DESERIALIZE_ARITHMETIC(TYPE, EXT)                                                        \
    template <>                                                                                          \
    struct ZenohDeserializer<TYPE> {                                                                     \
        static TYPE deserialize(const Bytes& b, ZResult* err = nullptr) {                                \
            TYPE t;                                                                                      \
            __ZENOH_RESULT_CHECK(::z_bytes_deserialize_into_##EXT(interop::as_loaned_c_ptr(b), &t), err, \
                                 "Failed to deserialize into " #TYPE);                                   \
            return t;                                                                                    \
        }                                                                                                \
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

auto make_slice(const uint8_t* data, size_t len) { return Slice{data, len}; };

template <class Deleter>
struct OwnedSlice {
    uint8_t* data;
    size_t len;
    Deleter d;
};

template <class Deleter>
auto make_owned_slice(uint8_t* data, size_t len, Deleter&& d) {
    return OwnedSlice<std::remove_reference_t<Deleter>>{data, len, std::forward<Deleter>(d)};
};

/// @brief Default codec for Zenoh serialization / deserialziation
class ZenohCodec {
    std::shared_ptr<void> _alias_guard;

   public:
    /// @brief Consturctor
    /// @param alias_guard optional alias guard. If null the data passed by const reference will copied,
    /// otherwise it will be aliased instead and a copy if alias_guard will be added to all serialized
    /// ``Bytes`` instances, to ensure that aliased data outlives them.
    ZenohCodec(std::shared_ptr<void> alias_guard = nullptr) : _alias_guard(std::move(alias_guard)) {}

    /// @brief Serialize pointer and length by copying.
    Bytes serialize(const Slice& s) const {
        Bytes b;
        if (this->_alias_guard == nullptr) {
            ::z_bytes_serialize_from_buf(interop::as_owned_c_ptr(b), s.data, s.len);
        } else {
            auto deleter = [ptr = this->_alias_guard](void*) mutable { ptr.reset(); };
            b = serialize(make_owned_slice(const_cast<uint8_t*>(s.data), s.len, std::move(deleter)));
        }
        return b;
    }

    /// @brief Serialize pointer and length by moving.
    template <class Deleter>
    Bytes serialize(OwnedSlice<Deleter>&& s) const {
        Bytes b;
        uint8_t* data = s.data;
        size_t len = s.len;
        auto d = [s_in = std::move(s)]() mutable { s_in.d(s_in.data); };
        using D = decltype(d);
        using Dval = std::remove_reference_t<D>;
        using DroppableType = typename detail::closures::Droppable<Dval>;
        auto drop = DroppableType::into_context(std::forward<D>(d));
        ::z_bytes_from_buf(interop::as_owned_c_ptr(b), data, len, detail::closures::_zenoh_drop_with_context, drop);
        return b;
    }

    Bytes serialize(std::string_view s) const {
        return serialize(Slice{reinterpret_cast<const uint8_t*>(s.data()), s.size()});
    }

    Bytes serialize(const char* s) const { return serialize(std::string_view(s)); }

    Bytes serialize(const std::string& s) const { return serialize(static_cast<std::string_view>(s)); }
    Bytes serialize(std::string&& s) const {
        std::string* ptr = new std::string(std::move(s));
        auto deleter = [p = ptr](void*) mutable { delete p; };
        return serialize(make_owned_slice(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(ptr->c_str())),
                                          ptr->size(), std::move(deleter)));
    }

    Bytes serialize(Bytes&& b) const { return std::move(b); }

    Bytes serialize(const Bytes& b) const { return b.clone(); }

#if (defined(SHARED_MEMORY) && defined(UNSTABLE))
    Bytes serialize(ZShm&& shm, ZResult* err = nullptr) const {
        Bytes b;
        __ZENOH_RESULT_CHECK(::z_bytes_serialize_from_shm(interop::as_owned_c_ptr(b), interop::as_moved_c_ptr(shm)),
                             err, "Failed to serialize ZShm");
        return b;
    }

    Bytes serialize(ZShmMut&& shm, ZResult* err = nullptr) const {
        Bytes b;
        __ZENOH_RESULT_CHECK(::z_bytes_serialize_from_shm_mut(interop::as_owned_c_ptr(b), interop::as_moved_c_ptr(shm)),
                             err, "Failed to serialize ZShmMut");
        return b;
    }
#endif

    template <class Allocator>
    Bytes serialize(const std::vector<uint8_t, Allocator>& s) const {
        auto b = ZenohCodec::serialize(make_slice(s.data(), s.size()));
        return b;
    }

    template <class Allocator>
    Bytes serialize(std::vector<uint8_t, Allocator>&& s) const {
        std::vector<uint8_t, Allocator>* ptr = new std::vector<uint8_t, Allocator>(std::move(s));
        auto deleter = [p = ptr](void*) mutable { delete p; };
        return serialize(make_owned_slice(ptr->data(), ptr->size(), std::move(deleter)));
    }

    template <class T, class Allocator>
    Bytes serialize(const std::vector<T, Allocator>& s) const {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class T, class Allocator>
    Bytes serialize(std::vector<T, Allocator>&& s) const {
        auto b =
            Bytes::serialize_from_iter(std::make_move_iterator(s.begin()), std::make_move_iterator(s.end()), *this);
        s.clear();
        return b;
    }

    template <class T, class Allocator>
    Bytes serialize(const std::deque<T, Allocator>& s) const {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class T, class Allocator>
    Bytes serialize(std::deque<T, Allocator>&& s) const {
        auto b =
            Bytes::serialize_from_iter(std::make_move_iterator(s.begin()), std::make_move_iterator(s.end()), *this);
        s.clear();
        return b;
    }

    template <class K, class H, class E, class Allocator>
    Bytes serialize(const std::unordered_set<K, H, E, Allocator>& s) {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class H, class E, class Allocator>
    Bytes serialize(std::unordered_set<K, H, E, Allocator>&& s) const {
        auto f = [&s](typename std::unordered_set<K, H, E, Allocator>::iterator it) -> K {
            return std::move(s.extract(it).value());
        };
        return Bytes::serialize_from_iter(detail::commons::make_transform_iterator(s.begin(), f),
                                          detail::commons::make_transform_iterator(s.end(), f), *this);
    }

    template <class K, class C, class Allocator>
    Bytes serialize(const std::set<K, C, Allocator>& s) const {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class C, class Allocator>
    Bytes serialize(std::set<K, C, Allocator>&& s) const {
        auto f = [&s](typename std::set<K, C, Allocator>::iterator it) -> K {
            return std::move(s.extract(it).value());
        };
        return Bytes::serialize_from_iter(detail::commons::make_transform_iterator(s.begin(), f),
                                          detail::commons::make_transform_iterator(s.end(), f), *this);
    }

    template <class K, class V, class H, class E, class Allocator>
    Bytes serialize(const std::unordered_map<K, V, H, E, Allocator>& s) const {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class V, class H, class E, class Allocator>
    Bytes serialize(std::unordered_map<K, V, H, E, Allocator>&& s) const {
        auto f = [&s](typename std::unordered_map<K, V, H, E, Allocator>::iterator it) -> std::pair<K, V> {
            auto node = s.extract(it);
            return std::pair<K, V>(std::move(node.key()), std::move(node.mapped()));
        };
        return Bytes::serialize_from_iter(detail::commons::make_transform_iterator(s.begin(), f),
                                          detail::commons::make_transform_iterator(s.end(), f), *this);
    }

    template <class K, class V, class C, class Allocator>
    Bytes serialize(const std::map<K, V, C, Allocator>& s) const {
        return Bytes::serialize_from_iter(s.begin(), s.end(), *this);
    }

    template <class K, class V, class C, class Allocator>
    Bytes serialize(std::map<K, V, C, Allocator>&& s) const {
        auto f = [&s](typename std::map<K, V, C, Allocator>::iterator it) -> std::pair<K, V> {
            auto node = s.extract(it);
            return std::pair<K, V>(std::move(node.key()), std::move(node.mapped()));
        };
        return Bytes::serialize_from_iter(detail::commons::make_transform_iterator(s.begin(), f),
                                          detail::commons::make_transform_iterator(s.end(), f), *this);
    }

    template <class A, class B>
    Bytes serialize(const std::pair<A, B>& s) const {
        auto ba = serialize(s.first);
        auto bb = serialize(s.second);
        Bytes b;
        ::z_bytes_from_pair(interop::as_owned_c_ptr(b), interop::as_moved_c_ptr(ba), interop::as_moved_c_ptr(bb));
        return b;
    }

    template <class A, class B>
    Bytes serialize(std::pair<A, B>&& s) const {
        auto ba = serialize(std::move(s.first));
        auto bb = serialize(std::move(s.second));
        Bytes b;
        ::z_bytes_from_pair(interop::as_owned_c_ptr(b), interop::as_moved_c_ptr(ba), interop::as_moved_c_ptr(bb));
        return b;
    }

    template <class T>
    Bytes serialize(std::shared_ptr<T> s) const {
        if (this->_alias_guard == nullptr) {
            T* ptr = s.get();
            ZenohCodec c(std::move(s));
            return c.serialize(*ptr);
        } else {
            // pointer is inside higher level pointer so there is no need to modify the alias guard.
            return this->serialize(*s.get());
        }
    }

#define __ZENOH_SERIALIZE_ARITHMETIC(TYPE, EXT)                        \
    Bytes serialize(TYPE t) const {                                    \
        Bytes b;                                                       \
        ::z_bytes_serialize_from_##EXT(interop::as_owned_c_ptr(b), t); \
        return b;                                                      \
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
    T deserialize(const Bytes& b, ZResult* err = nullptr) const {
        return detail::ZenohDeserializer<T>::deserialize(b, err);
    }
};

}  // namespace zenoh
