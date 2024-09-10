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

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An interface for accessing custom SHM segments
class CppShmSegment {
   public:
    virtual uint8_t* map(z_chunk_id_t chunk_id) = 0;
    virtual ~CppShmSegment() = default;
};

// Ensure that function pointers are defined with extern C linkage
namespace shm::segment::closures {
extern "C" {
inline uint8_t* _z_cpp_shm_segment_map_fn(z_chunk_id_t chunk, void* context) {
    return static_cast<CppShmSegment*>(context)->map(chunk);
}
inline void _z_cpp_shm_segment_drop_fn(void* context) { delete static_cast<CppShmSegment*>(context); }
}
}  // namespace shm::segment::closures

}  // end of namespace zenoh
