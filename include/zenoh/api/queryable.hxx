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

#include <utility>

#include "base.hxx"
#include "interop.hxx"

namespace zenoh {

class Session;
namespace detail {
class QueryableBase : public Owned<::z_owned_queryable_t> {
   protected:
    QueryableBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    QueryableBase(::z_owned_queryable_t* q) : Owned(q){};
    friend class zenoh::Session;
};
}  // namespace detail

template <class Handler>
class Queryable;

template <>
class Queryable<void> : public detail::QueryableBase {
    Queryable(zenoh::detail::null_object_t) : QueryableBase(zenoh::detail::null_object){};

   public:
    using QueryableBase::QueryableBase;
    friend class Session;
};

/// A Zenoh queryable. Constructed by ``Session::declare_queryable`` method.
/// @tparam Handler Streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class Queryable : public detail::QueryableBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream queryable from callback queryable and handler.
    ///
    /// @param q Callback queryable, that should expose data to the handler in its callback.
    /// @param handler Handler to access data exposed by q. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    Queryable(Queryable<void>&& q, Handler handler)
        : QueryableBase(interop::as_owned_c_ptr(q)), _handler(std::move(handler)) {}
    /// @name Methods

    /// @brief Return handler to queryable data stream.
    const Handler& handler() const { return _handler; };

    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned C representations of queryable and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_owned_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to owned C representations of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const Queryable<Handler>& q) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::QueryableBase&>(q)),
                          as_owned_c_ptr(q.handler()));
}

/// @brief Return a pair of pointers to loaned C representations of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_loaned_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to loaned C representation of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const Queryable<Handler>& q) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::detail::QueryableBase&>(q)),
                          as_loaned_c_ptr(q.handler()));
}

/// @brief Return a pair of pointers to moved C representation of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_moved_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to moved C representation of queryable and its handler.
/// Will return a pair of null pointers if optional is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<Queryable<Handler>>& q) -> decltype(as_moved_c_ptr(q.value())) {
    if (!q.has_value()) {
        return as_moved_c_ptr(q.value());
    } else {
        return {};
    }
}

/// @brief Move queryable and its handler to a pair containing corresponding Zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(Queryable<Handler>&& q) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::QueryableBase&>(q))),
                          move_to_c_obj(std::move(const_cast<Handler&>(q))));
}
}  // namespace interop

}  // namespace zenoh