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
#if (defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API))
#include "shm/buffer/buffer.hxx"
#endif

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace zenoh {

namespace detail::closures {
extern "C" {

inline void _zenoh_drop_with_context(void* data, void* context) {
    (void)data;
    IDroppable::delete_from_context(context);
}
}

}  // namespace detail::closures

struct Slice {
    const uint8_t* data;
    size_t len;
};

inline auto make_slice(const uint8_t* data, size_t len) { return Slice{data, len}; }

/// @brief A Zenoh data representation.
class Bytes : public Owned<::z_owned_bytes_t> {
   private:
   public:
    /// @name Constructors

    /// @brief Construct by copying sequence of bytes.
    template <class Allocator>
    Bytes(const std::vector<uint8_t, Allocator>& v) : Bytes() {
        ::z_bytes_copy_from_buf(interop::as_owned_c_ptr(*this), v.data(), v.size());
    }

    /// @brief Construct by moving sequence of bytes.
    template <class Allocator>
    Bytes(std::vector<uint8_t, Allocator>&& v) : Bytes() {
        std::vector<uint8_t, Allocator>* ptr = new std::vector<uint8_t, Allocator>(std::move(v));
        auto d = [p = ptr]() mutable { delete p; };
        using D = decltype(d);
        using Dval = std::remove_reference_t<D>;
        using DroppableType = typename detail::closures::Droppable<Dval>;
        auto drop = DroppableType::into_context(std::forward<D>(d));
        ::z_bytes_from_buf(interop::as_owned_c_ptr(*this), ptr->data(), ptr->size(),
                           detail::closures::_zenoh_drop_with_context, drop);
    }

    /// @brief Construct by copying sequence of charactes.
    Bytes(std::string_view v) : Bytes() {
        ::z_view_string_t s;
        z_view_string_from_substr(&s, v.data(), v.size());
        ::z_bytes_copy_from_string(interop::as_owned_c_ptr(*this), ::z_loan(s));
    }

    /// @brief Construct by copying sequence of charactes.
    Bytes(const char* v) : Bytes(std::string_view(v)){};

    /// @brief Construct by copying sequence of charactes.
    Bytes(const std::string& v) : Bytes(std::string_view(v)){};

    /// @brief Construct by moving a string.
    Bytes(std::string&& v) : Bytes() {
        std::string* ptr = new std::string(std::move(v));
        auto d = [p = ptr]() mutable { delete p; };
        using D = decltype(d);
        using Dval = std::remove_reference_t<D>;
        using DroppableType = typename detail::closures::Droppable<Dval>;
        auto drop = DroppableType::into_context(std::forward<D>(d));
        ::z_bytes_from_buf(interop::as_owned_c_ptr(*this), reinterpret_cast<uint8_t*>(ptr->data()), ptr->size(),
                           detail::closures::_zenoh_drop_with_context, drop);
    }

    /// @brief Construct by taking ownership of sequence of bytes.
    /// @tparam Deleter callable with signature void Deleter(uint8_t*).
    /// @param ptr pointer to data.
    /// @param len number of bytes to consider.
    /// @param deleter a thread-safe delete function to be invoked once corresponding ``Bytes`` object and all of its
    /// clones are destroyed.
    template <class Deleter>
    Bytes(uint8_t* ptr, size_t len, Deleter deleter) : Bytes() {
        static_assert(std::is_invocable_r<void, Deleter, uint8_t*>::value,
                      "deleter should be callable with the following signature: void deleter(uint8_t* data)");
        auto d = [p = ptr, del = std::move(deleter)]() mutable { del(p); };
        using D = decltype(d);
        using Dval = std::remove_reference_t<D>;
        using DroppableType = typename detail::closures::Droppable<Dval>;
        auto drop = DroppableType::into_context(std::forward<D>(d));
        ::z_bytes_from_buf(interop::as_owned_c_ptr(*this), ptr, len, detail::closures::_zenoh_drop_with_context, drop);
    }

    /// @brief Construct a shallow copy of this data.
    Bytes clone() const {
        Bytes b;
        ::z_bytes_clone(&b._0, interop::as_loaned_c_ptr(*this));
        return b;
    }

    /// @brief Construct an empty data.
    Bytes() : Owned(nullptr) { ::z_bytes_empty(interop::as_owned_c_ptr(*this)); }

#if (defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API))
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    Bytes(ZShm&& shm, ZResult* err = nullptr) : Bytes() {
        __ZENOH_RESULT_CHECK(::z_bytes_from_shm(interop::as_owned_c_ptr(*this), interop::as_moved_c_ptr(shm)), err,
                             "Failed to convert from ZShm");
    }

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    Bytes(ZShmMut&& shm, ZResult* err = nullptr) : Bytes() {
        __ZENOH_RESULT_CHECK(::z_bytes_from_shm_mut(interop::as_owned_c_ptr(*this), interop::as_moved_c_ptr(shm)), err,
                             "Failed to convert from ZShmMut");
    }
#endif

    /// @name Methods

    /// @brief Conver to vector of bytes.
    std::vector<uint8_t> as_vector() const {
        Reader r = this->reader();
        std::vector<uint8_t> v(this->size());
        r.read(v.data(), v.size());
        return v;
    }

    /// @brief Convert to string.
    std::string as_string() const {
        Reader r = this->reader();
        std::string s(this->size(), '\0');
        r.read(reinterpret_cast<uint8_t*>(s.data()), s.size());
        return s;
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Attempt to get a contiguous view to the underlying bytes
    ///
    /// This is only possible if data is not fragmented, otherwise the function will fail. In case of fragmented data,
    /// consider using ``Bytes::slice_iter``.
    /// @return A ``Slice`` containing pointer to underlying data and its length if data is non fragmented, an empty
    /// value otherwise.
    std::optional<Slice> get_contiguous_view() const {
        ::z_view_slice_t view;
        if (::z_bytes_get_contiguous_view(interop::as_loaned_c_ptr(*this), &view) == Z_OK) {
            return make_slice(::z_slice_data(z_loan(view)), ::z_slice_len(z_loan(view)));
        } else {
            return {};
        }
    }
#endif

#if (defined(Z_FEATURE_SHARED_MEMORY) && defined(Z_FEATURE_UNSTABLE_API))
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    std::optional<std::reference_wrapper<const ZShm>> as_shm() const {
        const z_loaned_shm_t* shm;
        if (::z_bytes_as_loaned_shm(interop::as_loaned_c_ptr(*this), &shm) != Z_OK) {
            return std::nullopt;
        }
        return std::cref(interop::as_owned_cpp_ref<const ZShm>(shm));
    }
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    std::optional<std::reference_wrapper<ZShm>> as_shm() {
        z_loaned_shm_t* shm;
        if (::z_bytes_as_mut_loaned_shm(interop::as_loaned_c_ptr(*this), &shm) != Z_OK) {
            return std::nullopt;
        }
        return std::ref(interop::as_owned_cpp_ref<ZShm>(shm));
    }
#endif

    /// @brief Get number of bytes in the pyload.
    size_t size() const { return ::z_bytes_len(interop::as_loaned_c_ptr(*this)); }

    class SliceIterator;

    /// @brief Returns an iterator on raw bytes slices.
    /// Zenoh may store data in non-contiguous regions of memory, this iterator
    /// then allows to access raw data directly without any attempt of deserializing it.
    /// Please note that no guarantee is provided on the internal memory layout.
    /// The only provided guarantee is on the bytes order that is preserved.
    SliceIterator slice_iter() const;

    /// @brief A reader for zenoh payload.
    class Reader : public Copyable<::z_bytes_reader_t> {
        using Copyable::Copyable;
        friend struct interop::detail::Converter;

       public:
        /// @name Constructors

        /// @brief Construct reader for specified data.
        Reader(const Bytes& b) : Copyable(::z_bytes_get_reader(interop::as_loaned_c_ptr(b))) {}
        /// @name Methods

        /// @brief Read data into specified destination.
        /// @param dst buffer where read data is written.
        /// @param len number of bytes to read.
        /// @return number of bytes that were read. Might be less than len if there is not enough data.
        size_t read(uint8_t* dst, size_t len) { return ::z_bytes_reader_read(&this->_0, dst, len); }

        /// @brief Return the read position indicator.
        /// @return read position indicator on success or -1L if failure occurs.
        int64_t tell() { return ::z_bytes_reader_tell(&this->_0); }

        /// @brief Return the number of bytes that can still be read.
        /// @return number of bytes that can still be read.
        size_t remaining() const { return ::z_bytes_reader_remaining(&this->_0); }

        /// @brief Set the reader position indicator to the value pointed to by offset, starting from the current
        /// position.
        /// @param offset offset in bytes starting from the current position.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_current(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_CUR), err, "seek_from_current failed");
        }

        /// @brief Set the reader position indicator to the value pointed to by offset, starting from the start of the
        /// data.
        /// @param offset offset in bytes starting from the 0-th byte position.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_start(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_SET), err, "seek_from_start failed");
        }

        /// @brief Set the reader position indicator to the value pointed to by offset with respect to the end of the
        /// data.
        /// @param offset offset in bytes starting from end position.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void seek_from_end(int64_t offset, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_reader_seek(&this->_0, offset, SEEK_END), err, "seek_from_end failed");
        }
    };

    /// @brief Create data reader.
    /// @return reader instance.
    Reader reader() const { return Reader(*this); }

    /// @brief A writer for zenoh payload.
    class Writer : public Owned<::z_owned_bytes_writer_t> {
       public:
        /// @name Constructors

        /// Construct an empty writer.
        Writer() : Owned(nullptr) { ::z_bytes_writer_empty(interop::as_owned_c_ptr(*this)); }

        /// @name Methods

        /// @brief Copy data from sepcified source into underlying ``Bytes`` instance.
        /// @param src source to copy data from.
        /// @param len number of bytes to copy from src to the underlying ``Bytes`` instance.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void write_all(const uint8_t* src, size_t len, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_write_all(interop::as_loaned_c_ptr(*this), src, len), err,
                                 "Failed to write data");
        }

        /// @brief Append another ``Bytes`` instance.
        /// This allows to compose data out of multiple ``Bytes`` that may point to different memory regions.
        /// Said in other terms, it allows to create a linear view on different memory regions without copy.
        ///
        /// @param data data to append.
        /// @param err if not null, the result code will be written to this location, otherwise ZException exception
        /// will be thrown in case of error.
        void append(Bytes&& data, ZResult* err = nullptr) {
            __ZENOH_RESULT_CHECK(::z_bytes_writer_append(interop::as_loaned_c_ptr(*this), z_move(data._0)), err,
                                 "Failed to append data");
        }

        /// @brief Finalize all writes and return underlying ``Bytes`` object.
        /// @return underlying ``Bytes`` object.
        Bytes finish() && {
            Bytes b;
            ::z_bytes_writer_finish(interop::as_moved_c_ptr(*this), interop::as_owned_c_ptr(b));
            return b;
        }
    };
};

/// @brief An iterator over raw bytes slices.
class Bytes::SliceIterator : Copyable<::z_bytes_slice_iterator_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Return next raw slice of serialized data.
    /// @return next raw slice of serialized data. If the iterator reached the end, an empty optional will be returned.
    std::optional<Slice> next() {
        ::z_view_slice_t s;
        if (!::z_bytes_slice_iterator_next(&this->_0, &s)) {
            return {};
        }
        return Slice{::z_slice_data(z_loan(s)), ::z_slice_len(z_loan(s))};
    }
};

inline Bytes::SliceIterator Bytes::slice_iter() const {
    return interop::into_copyable_cpp_obj<Bytes::SliceIterator>(
        ::z_bytes_get_slice_iterator(interop::as_loaned_c_ptr(*this)));
}

}  // namespace zenoh
