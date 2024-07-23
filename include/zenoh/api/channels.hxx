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
#include "../detail/interop.hxx"
#include "base.hxx"
#include "sample.hxx"
#include "reply.hxx"
#include "query.hxx"

namespace zenoh::channels {

namespace detail {
    template<class T>
    struct FifoHandlerData {};

    template<>
    struct FifoHandlerData<zenoh::Sample> {
        typedef ::z_owned_fifo_handler_sample_t handler_type;
        typedef ::z_owned_closure_sample_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_fifo_channel_sample_new(cb, h, capacity);
        }
    };

    template<>
    struct FifoHandlerData<zenoh::Query> {
        typedef ::z_owned_fifo_handler_query_t handler_type;
        typedef ::z_owned_closure_query_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_fifo_channel_query_new(cb, h, capacity);
        }
    };

    template<>
    struct FifoHandlerData<zenoh::Reply> {
        typedef ::z_owned_fifo_handler_reply_t handler_type;
        typedef ::z_owned_closure_reply_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_fifo_channel_reply_new(cb, h, capacity);
        }
    };

    template<class T>
    struct RingHandlerData {};

    template<>
    struct RingHandlerData<zenoh::Sample> {
        typedef ::z_owned_ring_handler_sample_t handler_type;
        typedef ::z_owned_closure_sample_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_ring_channel_sample_new(cb, h, capacity);
        }
    };

    template<>
    struct RingHandlerData<zenoh::Query> {
        typedef ::z_owned_ring_handler_query_t handler_type;
        typedef ::z_owned_closure_query_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_ring_channel_query_new(cb, h, capacity);
        }
    };

    template<>
    struct RingHandlerData<zenoh::Reply> {
        typedef ::z_owned_ring_handler_reply_t handler_type;
        typedef ::z_owned_closure_reply_t closure_type;
        static void create_cb_handler_pair(closure_type* cb, handler_type* h, size_t capacity) {
            ::z_ring_channel_reply_new(cb, h, capacity);
        }
    };
}

/// @brief A FIFO channel handler
/// @tparam T data entry type
template<class T>
class FifoHandler : public Owned<typename detail::FifoHandlerData<T>::handler_type> {
public:
    using Owned<typename detail::FifoHandlerData<T>::handler_type>::Owned;

    /// @name Methods

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty will block until new data entry arrives.
    /// @return a pair containing a received data entry (will be in a gravestone state if there is no more data in the buffer and the stream is inactive),
    /// and a bool flag indicating whether handler's stream is still active, i.e. if there is still possibility to fetch more data in the future. 
    std::pair<T, bool> recv() const {
        std::pair<T, bool> p(nullptr, false);
        p.second = ::z_recv(this->loan(), zenoh::detail::as_owned_c_ptr(p.first));
        return p;
    }

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty will immediately return (with data entry in a gravestone state). 
    /// @return a pair containing a received data entry (will be in null state if there is no more data in the buffer),
    /// and a bool flag indicating whether handler's stream is still active, i.e. if there is still possibility to fetch more data in the future.
    std::pair<T, bool> try_recv() const {
        std::pair<T, bool> p(nullptr, false);
        p.second = ::z_try_recv(this->loan(), zenoh::detail::as_owned_c_ptr(p.first));
        return p;
    }
};

/// @brief A circular buffer channel handler.
/// @tparam T data entry type.
template<class T>
class RingHandler : public Owned<typename detail::RingHandlerData<T>::handler_type> {
public:
    using Owned<typename detail::RingHandlerData<T>::handler_type>::Owned;

    /// @name Methods

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty will block until new data entry arrives
    /// @return a pair containing a received data entry (will be in gravestone state if there is no more data in the buffer and the stream is inactive),
    /// and a bool flag indicating whether handler's stream is still active, i.e. if there is still possibility to fetch more data in the future. 
    std::pair<T, bool> recv() const {
        std::pair<T, bool> p(nullptr, false);
        p.second = ::z_recv(this->loan(), zenoh::detail::as_owned_c_ptr(p.first));
        return p;
    }

    /// @brief Fetch a data entry from the handler's buffer. If buffer is empty will immediately return (with data entry in a gravestone state). 
    /// @return a pair containing a received data entry (will be in null state if there is no more data in the buffer),
    /// and a bool flag indicating whether handler's stream is still active, i.e. if there is still possibility to fetch more data in the future. 
    std::pair<T, bool> try_recv() const {
        std::pair<T, bool> p(nullptr, false);
        p.second = ::z_try_recv(this->loan(), zenoh::detail::as_owned_c_ptr(p.first));
        return p;
    }
};

/// @brief A FIFO channel.
class FifoChannel {
    size_t _capacity;
public:
    /// @brief Constructor.
    /// @param capacity maximum number of entries in the FIFO buffer of the channel. When the buffer is full all
    /// new incoming entries will be ignored.
    FifoChannel(size_t capacity)
        :_capacity(capacity)
    {}

    /// @brief Channel handler type.
    template<class T>
    using HandlerType = FifoHandler<T>;

    /// @internal
    /// @brief Convert channel into a pair of zenoh callback and handler for the specified type.
    /// @tparam T entry type.
    /// @return a callback-handler pair.
    template<class T>
    std::pair<typename detail::FifoHandlerData<T>::closure_type, HandlerType<T>> into_cb_handler_pair() const {
        typename detail::FifoHandlerData<T>::closure_type c_closure;
        FifoHandler<T> h(nullptr);
        detail::FifoHandlerData<T>::create_cb_handler_pair(&c_closure, zenoh::detail::as_owned_c_ptr(h), _capacity);
        return {c_closure, std::move(h)};
    }
};

/// @brief A circular buffer channel.
class RingChannel {
    size_t _capacity;
public:
    /// @brief Constructor.
    /// @param capacity  maximum number of entries in circular buffer of the channel. When the buffer is full, the older entries
    /// will be removed to provide room for the new ones.
    RingChannel(size_t capacity)
        :_capacity(capacity)
    {}

    /// @brief Channel handler type.
    template<class T>
    using HandlerType = RingHandler<T>;

    /// @internal
    /// @brief Convert channel into a pair of zenoh callback and handler for the specified type.
    /// @tparam T entry type.
    /// @return a callback-handler pair.
    template<class T>
    std::pair<typename detail::RingHandlerData<T>::closure_type, HandlerType<T>> into_cb_handler_pair() const {
        typename detail::RingHandlerData<T>::closure_type c_closure;
        RingHandler<T> h(nullptr);
        detail::RingHandlerData<T>::create_cb_handler_pair(&c_closure, zenoh::detail::as_owned_c_ptr(h), _capacity);
        return {c_closure, std::move(h)};
    }
};

}