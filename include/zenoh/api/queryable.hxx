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

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERYABLE == 1

#include <utility>

#include "base.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"

#if defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif

namespace zenoh {

class Session;
namespace detail {
class QueryableBase : public Owned<::z_owned_queryable_t> {
   protected:
    QueryableBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    QueryableBase(::z_owned_queryable_t* q) : Owned(q){};
    friend class zenoh::Session;

   public:
    /// @brief Get the key expression of the queryable.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_queryable_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the queryable.
    /// @return id of this queryable.
    EntityGlobalId get_id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(::z_queryable_id(interop::as_loaned_c_ptr(*this)));
    }
#endif
};
}  // namespace detail

template <class Handler>
class Queryable;

template <>
class Queryable<void> : public detail::QueryableBase {
    Queryable(zenoh::detail::null_object_t) : QueryableBase(zenoh::detail::null_object){};

   public:
    /// @name Methods

    /// @brief Undeclare queryable.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_queryable(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare queryable");
    }
    friend class Session;
};

/// @brief A Zenoh queryable. Destroying or undeclaring queryable cancels its callback function.
///
/// Constructed by ``Session::declare_queryable`` method.
/// @tparam Handler streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class Queryable : public detail::QueryableBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream queryable from callback queryable and handler.
    ///
    /// @param q Callback queryable, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by q. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    Queryable(Queryable<void>&& q, Handler handler)
        : QueryableBase(interop::as_owned_c_ptr(q)), _handler(std::move(handler)) {}
    /// @name Methods

    /// @brief Return handler to queryable data stream.
    const Handler& handler() const { return _handler; };

    /// @brief Undeclare queryable, and return its handler, which can still be used to examine any queries
    /// received prior to undeclaration, replying to such queries is undefined behaviour.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_queryable(::z_move(this->_0)), err, "Failed to undeclare queryable");
        return std::move(this->_handler);
    }

    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of queryable and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_owned_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const Queryable<Handler>& q) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::QueryableBase&>(q)),
                          as_owned_c_ptr(q.handler()));
}

/// @brief Return a pair of pointers to loaned zenoh-c representations of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_loaned_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_loaned_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to loaned zenoh-c representation of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_loaned_c_ptr(const Queryable<Handler>& q) {
    return std::make_pair(as_loaned_c_ptr(static_cast<const zenoh::detail::QueryableBase&>(q)),
                          as_loaned_c_ptr(q.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representation of queryable and its handler.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(Queryable<Handler>& q) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::QueryableBase&>(q)),
                          as_moved_c_ptr(const_cast<Handler&>(q.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representation of queryable and its handler.
/// Will return a pair of null pointers if optional is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<Queryable<Handler>>& q) -> decltype(as_moved_c_ptr(q.value())) {
    if (!q.has_value()) {
        return as_moved_c_ptr(q.value());
    } else {
        return {};
    }
}

/// @brief Move queryable and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(Queryable<Handler>&& q) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::QueryableBase&>(q))),
                          move_to_c_obj(std::move(const_cast<Handler&>(q))));
}
}  // namespace interop

}  // namespace zenoh
#endif