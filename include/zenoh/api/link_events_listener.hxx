//
// Copyright (c) 2026 ZettaScale Technology
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

#if defined(Z_FEATURE_UNSTABLE_API)

#include <utility>

#include "base.hxx"
#include "interop.hxx"

namespace zenoh {

class Session;

namespace detail {
class LinkEventsListenerBase : public Owned<::z_owned_link_events_listener_t> {
   protected:
    LinkEventsListenerBase(zenoh::detail::null_object_t) : Owned(nullptr) {}
    LinkEventsListenerBase(::z_owned_link_events_listener_t* l) : Owned(l) {}
    friend struct interop::detail::Converter;
};
}  // namespace detail

template <class Handler>
class LinkEventsListener;

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A callback-only link events listener.
template <>
class LinkEventsListener<void> : public detail::LinkEventsListenerBase {
   protected:
    using LinkEventsListenerBase::LinkEventsListenerBase;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Undeclare link events listener.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_link_events_listener(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare link events listener");
    }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A Zenoh link events listener.
///
/// Constructed by ``Session::declare_link_events_listener`` method.
/// @tparam Handler streaming handler exposing data. If `void`, no handler access is provided and instead data is being
/// processed inside the callback.
template <class Handler>
class LinkEventsListener : public detail::LinkEventsListenerBase {
    Handler _handler;

   public:
    /// @name Constructors

    /// @brief Construct stream link events listener from callback listener and handler.
    ///
    /// @param l callback link events listener, that should expose data to the handler in its callback.
    /// @param handler handler to access data exposed by `l`.
    LinkEventsListener(LinkEventsListener<void>&& l, Handler handler)
        : LinkEventsListenerBase(interop::as_owned_c_ptr(l)), _handler(std::move(handler)) {}

    /// @name Methods

    /// @brief Undeclare link events listener, and return its handler.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    Handler undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_link_events_listener(::z_move(this->_0)), err,
                             "Failed to undeclare link events listener");
        return std::move(this->_handler);
    }

    /// @brief Return the handler to link events data stream.
    const Handler& handler() const { return _handler; };
    friend class Session;
};

namespace interop {
/// @brief Return a pair of pointers to owned zenoh-c representations of link events listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(LinkEventsListener<Handler>& l) {
    return std::make_pair(as_owned_c_ptr(static_cast<zenoh::detail::LinkEventsListenerBase&>(l)),
                          as_owned_c_ptr(const_cast<Handler&>(l.handler())));
}

/// @brief Return a pair of pointers to owned zenoh-c representations of link events listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_owned_c_ptr(const LinkEventsListener<Handler>& l) {
    return std::make_pair(as_owned_c_ptr(static_cast<const zenoh::detail::LinkEventsListenerBase&>(l)),
                          as_owned_c_ptr(l.handler()));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of link events listener and its callback.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(LinkEventsListener<Handler>& l) {
    return std::make_pair(as_moved_c_ptr(static_cast<zenoh::detail::LinkEventsListenerBase&>(l)),
                          as_moved_c_ptr(const_cast<Handler&>(l.handler())));
}

/// @brief Return a pair of pointers to moved zenoh-c representations of link events listener and its callback.
/// Will return a pair of null pointers if option is empty.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto as_moved_c_ptr(std::optional<LinkEventsListener<Handler>>& l) -> decltype(as_moved_c_ptr(l.value())) {
    if (l.has_value()) {
        return as_moved_c_ptr(l.value());
    } else {
        return {};
    }
}

/// @brief Move link events listener and its handler to a pair containing corresponding zenoh-c structs.
template <class Handler, typename = std::enable_if_t<!std::is_same_v<Handler, void>>>
auto move_to_c_obj(LinkEventsListener<Handler>&& l) {
    return std::make_pair(move_to_c_obj(std::move(static_cast<zenoh::detail::LinkEventsListenerBase&>(l))),
                          move_to_c_obj(std::move(const_cast<Handler&>(l))));
}
}  // namespace interop

}  // namespace zenoh
#endif
