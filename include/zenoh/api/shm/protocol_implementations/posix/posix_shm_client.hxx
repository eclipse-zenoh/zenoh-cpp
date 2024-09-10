//
// Copyright (c) 2023 ZettaScale Technology
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
#pragma once

#include "../../client/shm_client.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Client factory implementation for particular shared memory protocol
class PosixShmClient : public ShmClient {
   public:
    /// @name Constructors

    /// @brief Create a new PosixShmClient.
    PosixShmClient() : ShmClient(nullptr) { ::z_posix_shm_client_new(&this->_0); }
};

}  // end of namespace zenoh
