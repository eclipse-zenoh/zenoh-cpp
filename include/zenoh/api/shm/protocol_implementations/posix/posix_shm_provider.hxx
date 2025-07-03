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

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An SHM provider implementing zenoh-standard POSIX shared memory protocol
class PosixShmProvider : public ShmProvider {
   public:
    using ShmProvider::ShmProvider;

    /// @name Constructors

    /// @brief Create a new PosixShmProvider.
    /// @param size size of POSIX shared memory segment to be allocated and used by the provider
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    PosixShmProvider(std::size_t size, ZResult* err = nullptr) : ShmProvider(zenoh::detail::null_object) {
        __ZENOH_RESULT_CHECK(::z_posix_shm_provider_new(&this->_0, size), err, "Failed to create POSIX SHM provider");
    }

    /// @brief Create a new PosixShmProvider.
    /// @param layout layout for POSIX shared memory segment to be allocated and used by the provider
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    PosixShmProvider(const MemoryLayout& layout, ZResult* err = nullptr) : ShmProvider(zenoh::detail::null_object) {
        __ZENOH_RESULT_CHECK(::z_posix_shm_provider_with_layout_new(&this->_0, interop::as_loaned_c_ptr(layout)), err,
                             "Failed to create POSIX SHM provider");
    }
};

}  // end of namespace zenoh
