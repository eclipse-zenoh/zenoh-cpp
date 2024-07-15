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
#include "keyexpr.hxx"

namespace zenoh {

class SubscriberBase : public Owned<::z_owned_subscriber_t> {
public:
    using Owned::Owned;

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get the key expression of the subscriber
    /// @note zenoh-c only.
    const KeyExpr& get_keyexpr() const { 
        return detail::as_owned_cpp_obj<KeyExpr>(::z_subscriber_keyexpr(this->loan())); 
    }
#endif
};

/// A Zenoh subscriber. Destroying subscriber cancels the subscription.
/// Constructed by ``Session::declare_subscriber`` method.
template<class Handler>
class Subscriber: public SubscriberBase {
    Handler _handler;
public:
    /// @name Constructors

    /// @internal
    /// @brief Construct from subscriber and handler.
    Subscriber(SubscriberBase subscriber, Handler handler)
        :SubscriberBase(std::move(subscriber)), _handler(std::move(handler)) {
    }

    /// @name Methods

#ifdef ZENOHCXX_ZENOHC
    using SubscriberBase::get_keyexpr;
#endif
    /// @brief Return the handler to subscriber data stream.
    const Handler& handler() const { return _handler; };
};

template<>
class Subscriber<void> :public SubscriberBase {
public:
    using SubscriberBase::SubscriberBase;
};


}