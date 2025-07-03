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

#include <memory.h>

#include "../../base.hxx"
#include "../common/types.hxx"
#include "shm_segment.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An interface for making custom SHM clients
class CppShmClient {
   public:
    virtual std::unique_ptr<CppShmSegment> attach(SegmentId segment_id) = 0;
    virtual ProtocolId id() const = 0;
    virtual ~CppShmClient() = default;
};

// Ensure that function pointers are defined with extern C linkage
namespace shm::client::closures {
extern "C" {
inline bool _z_cpp_shm_client_attach_fn(struct z_shm_segment_t* out_segment, z_segment_id_t id, void* context) {
    if (auto segment = static_cast<CppShmClient*>(context)->attach(id)) {
        out_segment->context.context.ptr = segment.release();
        out_segment->context.delete_fn = &shm::segment::closures::_z_cpp_shm_segment_drop_fn;
        out_segment->callbacks.map_fn = &shm::segment::closures::_z_cpp_shm_segment_map_fn;
        return true;
    }
    return false;
}

inline ProtocolId _z_cpp_shm_client_id_fn(void* context) { return static_cast<CppShmClient*>(context)->id(); }

inline void _z_cpp_shm_client_drop_fn(void* context) { delete static_cast<CppShmClient*>(context); }
}
}  // namespace shm::client::closures

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An SHM client for reading shared memory buffers
class ShmClient : public Owned<::z_owned_shm_client_t> {
    friend class ShmClientStorage;

   public:
    /// @name Constructors

    /// @brief Create a new CPP-defined ShmClient.
    ShmClient(std::unique_ptr<CppShmClient>&& cpp_interface) : Owned(nullptr) {
        zc_threadsafe_context_t context = {{cpp_interface.release()},
                                           &shm::client::closures::_z_cpp_shm_client_drop_fn};
        zc_shm_client_callbacks_t callbacks = {
            &shm::client::closures::_z_cpp_shm_client_attach_fn,
            &shm::client::closures::_z_cpp_shm_client_id_fn,
        };
        z_shm_client_new(&this->_0, context, callbacks);
    }
};

}  // end of namespace zenoh
