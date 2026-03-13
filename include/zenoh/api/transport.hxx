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

#include "../zenohc.hxx"
#include "base.hxx"
#include "enums.hxx"
#include "id.hxx"
#include "interop.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Represents a connection to a remote zenoh node.
class Transport : public Owned<::z_owned_transport_t> {
   public:
    /// @name Methods

    /// @brief Get the Zenoh ID of the remote node.
    /// @return `Id` of the remote node.
    Id get_zid() const {
        return interop::into_copyable_cpp_obj<Id>(::z_transport_zid(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Get the type of the remote node.
    /// @return `zenoh::WhatAmI` of the remote node.
    WhatAmI get_whatami() const { return ::z_transport_whatami(interop::as_loaned_c_ptr(*this)); }

    /// @brief Check if the transport supports QoS.
    /// @return `true` if QoS is supported, `false` otherwise.
    bool is_qos() const { return ::z_transport_is_qos(interop::as_loaned_c_ptr(*this)); }

    /// @brief Check if the transport is multicast.
    /// @return `true` if the transport is multicast, `false` otherwise.
    bool is_multicast() const { return ::z_transport_is_multicast(interop::as_loaned_c_ptr(*this)); }

#if defined(Z_FEATURE_SHARED_MEMORY)
    /// @brief Check if the transport supports shared memory.
    /// @return `true` if shared memory is supported, `false` otherwise.
    bool is_shm() const { return ::z_transport_is_shm(interop::as_loaned_c_ptr(*this)); }
#endif

    /// @brief Copy constructor.
    Transport(const Transport& other) : Owned(nullptr) {
        ::z_transport_clone(&this->_0, interop::as_loaned_c_ptr(other));
    }

    /// @brief Move constructor.
    Transport(Transport&& other) = default;

    /// @name Operators

    /// @brief Assignment operator.
    Transport& operator=(const Transport& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_transport_clone(&this->_0, interop::as_loaned_c_ptr(other));
        }
        return *this;
    }

    /// @brief Move assignment operator.
    Transport& operator=(Transport&& other) = default;
};

}  // namespace zenoh
#endif
