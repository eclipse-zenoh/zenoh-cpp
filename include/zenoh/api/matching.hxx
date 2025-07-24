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
#include "base.hxx"
#include "interop.hxx"

namespace zenoh {
class Publisher;
class Querier;

/// @brief A struct that indicates if there exist Subscribers matching the Publisher's key expression or Queryables
/// matching Querier's key expression and target.
struct MatchingStatus {
    /// @name Fields

    /// true if there exist entities matching the target (i.e either Subscribers matching Publisher's key expression or
    /// Queryables matching Querier's key expression and target).
    bool matching;
};

namespace detail::closures {
extern "C" {
inline void _zenoh_on_status_change_call(const ::z_matching_status_t* status, void* context) {
    IClosure<void, const MatchingStatus&>::call_from_context(context, MatchingStatus{status->matching});
}
}
}  // namespace detail::closures

namespace detail {
class MatchingListenerBase : public Owned<::z_owned_matching_listener_t> {
   protected:
    MatchingListenerBase(zenoh::detail::null_object_t) : Owned(nullptr){};
    MatchingListenerBase(::z_owned_matching_listener_t* m) : Owned(m){};
    friend struct interop::detail::Converter;
};
}  // namespace detail

template <class Handler>
class MatchingListener;

template <>
class MatchingListener<void> : public detail::MatchingListenerBase {
   protected:
    using MatchingListenerBase::MatchingListenerBase;
    friend class Publisher;
    friend class Querier;

   public:
    /// @name Methods

    /// @brief Undeclare matching listener.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_matching_listener(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare matching listener");
    }
};

/// @brief A Zenoh matching listener.
///
/// A listener that sends notifications when the [`MatchingStatus`] of a corresponding Zenoh entity changes.
/// Dropping the corresponding Zenoh entity, disables the matching listener.
/// @note Zenoh-c only.
template <class Handler>
class MatchingListener : public detail::MatchingListenerBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream matching listener from callback matching listener and handler.
    ///
    /// @param m callback matching listener, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `m`. Zenoh handlers implement
    /// recv and try_recv methods, for blocking and non-blocking message reception. But user is free to define his own
    /// interface.
    MatchingListener(MatchingListener<void>&& m, Handler handler)
        : MatchingListenerBase(interop::as_owned_c_ptr(m)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare matching listener, and return its handler, which can still be used to process any messages
    /// received prior to undeclaration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_matching_listener(::z_move(this->_0)), err,
                             "Failed to undeclare matching listener");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to matching listener data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of matching listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(MatchingListener<Handler>& m) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::MatchingListenerBase&>(m)),
                          as_owned_c_ptr(const_cast<Handler&>(m.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of matching listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const MatchingListener<Handler>& m) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::MatchingListenerBase&>(m)),
                          as_owned_c_ptr(m.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of matching listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(MatchingListener<Handler>& m) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::MatchingListenerBase&>(m)),
                          as_moved_c_ptr(const_cast<Handler&>(m.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of matching listener and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<MatchingListener<Handler>>& m) -> decltype(as_moved_c_ptr(m.value())) {
    if (!m.has_value()) {
        return as_moved_c_ptr(m.value());
    } else {
        return {};
    }
}

/// @brief Move matching listener and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(MatchingListener<Handler>&& m) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::MatchingListenerBase&>(m))),
                          move_to_c_obj(std::move(const_cast<Handler&>(m))));
}
}  // namespace interop

}  // namespace zenoh
