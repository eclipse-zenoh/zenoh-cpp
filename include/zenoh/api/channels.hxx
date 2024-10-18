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

//
// This file contains structures and classes API without implementations
//

#pragma once
#include <variant>

#include "base.hxx"
#include "interop.hxx"
#include "query.hxx"
#include "reply.hxx"
#include "sample.hxx"

namespace zenoh::channels {

/// @brief Error of the `recv` or `try_recv` operation.
enum class RecvError {
    /// @brief Channel is closed and will no more receive any data.
    Z_DISCONNECTED = Z_CHANNEL_DISCONNECTED,
    /// @brief Channel is still active, but no data is currently available in its buffer,
    /// future calls to `try_recv` might still succeed.
    Z_NODATA = Z_CHANNEL_NODATA
};

namespace detail {
template <class T>
struct FifoHandlerData {};

template <>
struct FifoHandlerData<zenoh::Sample> {
    typedef ::z_owned_fifo_handler_sample_t handler_type;
    typedef ::z_owned_closure_sample_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_fifo_channel_sample_new(cb, h, capacity);
    }
};

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1
template <>
struct FifoHandlerData<zenoh::Query> {
    typedef ::z_owned_fifo_handler_query_t handler_type;
    typedef ::z_owned_closure_query_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_fifo_channel_query_new(cb, h, capacity);
    }
};
#endif

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
template <>
struct FifoHandlerData<zenoh::Reply> {
    typedef ::z_owned_fifo_handler_reply_t handler_type;
    typedef ::z_owned_closure_reply_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_fifo_channel_reply_new(cb, h, capacity);
    }
};
#endif

template <class T>
struct RingHandlerData {};

template <>
struct RingHandlerData<zenoh::Sample> {
    typedef ::z_owned_ring_handler_sample_t handler_type;
    typedef ::z_owned_closure_sample_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_ring_channel_sample_new(cb, h, capacity);
    }
};

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1
template <>
struct RingHandlerData<zenoh::Query> {
    typedef ::z_owned_ring_handler_query_t handler_type;
    typedef ::z_owned_closure_query_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_ring_channel_query_new(cb, h, capacity);
    }
};
#endif

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
template <>
struct RingHandlerData<zenoh::Reply> {
    typedef ::z_owned_ring_handler_reply_t handler_type;
    typedef ::z_owned_closure_reply_t closure_type;
    static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
        ::z_ring_channel_reply_new(cb, h, capacity);
    }
};
#endif
}  // namespace detail

class FifoChannel;

/// @brief A FIFO channel handler.
/// @tparam T data entry type.
template <class T>
class FifoHandler : public Owned<typename detail::FifoHandlerData<T>::handler_type> {
    FifoHandler(zenoh::detail::null_object_t) : Owned<typename detail::FifoHandlerData<T>::handler_type>(nullptr){};

   public:
    /// @name Methods

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty, will block until new data entry
    /// arrives.
    /// @return received data entry, if there were any in the buffer, a receive error otherwise.
    std::variant<T, RecvError> recv() const {
        std::variant<T, RecvError> v(interop::detail::null<T>());
        z_result_t res = ::z_recv(interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(std::get<T>(v)));
        if (res == Z_OK) {
            return v;
        } else {
            return RecvError::Z_DISCONNECTED;
        }
    }

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty, will immediately return.
    /// @return received data entry, if there were any in the buffer, a receive error otherwise.
    std::variant<T, RecvError> try_recv() const {
        std::variant<T, RecvError> v(interop::detail::null<T>());
        z_result_t res = ::z_try_recv(interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(std::get<T>(v)));
        if (res == Z_OK) {
            return v;
        } else if (res == Z_CHANNEL_NODATA) {
            return RecvError::Z_NODATA;
        } else {
            return RecvError::Z_DISCONNECTED;
        }
    }

    friend class FifoChannel;
};

class RingChannel;

/// @brief A circular buffer channel handler.
/// @tparam T data entry type.
template <class T>
class RingHandler : public Owned<typename detail::RingHandlerData<T>::handler_type> {
    RingHandler(zenoh::detail::null_object_t) : Owned<typename detail::RingHandlerData<T>::handler_type>(nullptr){};

   public:
    /// @name Methods

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty, will block until new data entry
    /// arrives.
    /// @return received data, entry if there were any in the buffer, a receive error otherwise.
    std::variant<T, RecvError> recv() const {
        std::variant<T, RecvError> v(interop::detail::null<T>());
        z_result_t res =
            ::z_recv(zenoh::interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(std::get<T>(v)));
        if (res == Z_OK) {
            return v;
        } else {
            return RecvError::Z_DISCONNECTED;
        }
    }

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty, will immediately return.
    /// @return received data entry, if there were any in the buffer, a receive error otherwise.
    std::variant<T, RecvError> try_recv() const {
        std::variant<T, RecvError> v(interop::detail::null<T>());
        z_result_t res = ::z_try_recv(interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(std::get<T>(v)));
        if (res == Z_OK) {
            return v;
        } else if (res == Z_CHANNEL_NODATA) {
            return RecvError::Z_NODATA;
        } else {
            return RecvError::Z_DISCONNECTED;
        }
    }

    friend class RingChannel;
};

/// @brief A FIFO channel.
class FifoChannel {
    size_t _capacity;

   public:
    /// @brief Constructor.
    /// @param capacity maximum number of entries in the FIFO buffer of the channel. When the buffer is full, all
    /// new attempts to insert data will block, until an entry is fetched and the space is freed in the buffer.
    FifoChannel(size_t capacity) : _capacity(capacity) {}

    /// @brief Channel handler type.
    template <class T>
    using HandlerType = FifoHandler<T>;

    /// @internal
    /// @brief Convert channel into a pair of zenoh callback and handler for the specified type.
    /// @tparam T entry type.
    /// @return a callback-handler pair.
    template <class T>
    std::pair<typename detail::FifoHandlerData<T>::closure_type, HandlerType<T>> into_cb_handler_pair() const {
        typename detail::FifoHandlerData<T>::closure_type c_closure;
        FifoHandler<T> h(zenoh::detail::null_object);
        detail::FifoHandlerData<T>::create_cb_handler_pair(&c_closure, zenoh::interop::as_owned_c_ptr(h), _capacity);
        return {c_closure, std::move(h)};
    }
};

/// @brief A circular buffer channel.
class RingChannel {
    size_t _capacity;

   public:
    /// @brief Constructor.
    /// @param capacity  maximum number of entries in circular buffer of the channel. When the buffer is full, the older
    /// entries will be removed to provide room for the new ones.
    RingChannel(size_t capacity) : _capacity(capacity) {}

    /// @brief Channel handler type.
    template <class T>
    using HandlerType = RingHandler<T>;

    /// @internal
    /// @brief Convert channel into a pair of zenoh callback and handler for the specified type.
    /// @tparam T entry type.
    /// @return a callback-handler pair.
    template <class T>
    std::pair<typename detail::RingHandlerData<T>::closure_type, HandlerType<T>> into_cb_handler_pair() const {
        typename detail::RingHandlerData<T>::closure_type c_closure;
        RingHandler<T> h(zenoh::detail::null_object);
        detail::RingHandlerData<T>::create_cb_handler_pair(&c_closure, zenoh::interop::as_owned_c_ptr(h), _capacity);
        return {c_closure, std::move(h)};
    }
};

}  // namespace zenoh::channels