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

#include "../../provider/shm_provider.hxx"

namespace zenoh {

/// @brief An SHM provider implementing zenoh-standard POSIX shared memory protocol
class PosixShmProvider : public ShmProvider {
   public:
    using ShmProvider::ShmProvider;

    /// @name Constructors

    /// @brief Create a new PosixShmProvider.
    /// @param layout layout for POSIX shared memory segment to be allocated and used by the provider
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    PosixShmProvider(const MemoryLayout& layout, ZResult* err = nullptr) : ShmProvider(nullptr) {
        __ZENOH_RESULT_CHECK(::z_posix_shm_provider_new(&this->_0, layout.loan()), err,
                             "Failed to create POSIX SHM provider");
    }
};

}  // end of namespace zenoh
