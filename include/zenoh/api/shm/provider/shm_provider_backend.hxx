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

#include "../../../detail/interop.hxx"
#include "../../base.hxx"
#include "chunk.hxx"
#include "types.hxx"

namespace zenoh {

class CppShmProviderBackendIface {
   public:
    virtual ChunkAllocResult alloc(const MemoryLayout &layout) = 0;
    virtual void free(const ChunkDescriptor &chunk) = 0;
    virtual size_t defragment() = 0;
    virtual size_t available() const = 0;
    virtual void layout_for(MemoryLayout &layout) = 0;
    virtual ~CppShmProviderBackendIface() = default;
};

class CppShmProviderBackend : public CppShmProviderBackendIface {};

class CppShmProviderBackendThreadsafe : public CppShmProviderBackend {};

// Ensure that function pointers are defined with extern C linkage
namespace shm::provider_backend::closures {
extern "C" {
inline void del(void *context) { delete static_cast<CppShmProviderBackend *>(context); }
inline void alloc_fn(struct z_owned_chunk_alloc_result_t *out_result, const struct z_loaned_memory_layout_t *layout,
                     void *context) {
    *out_result = static_cast<CppShmProviderBackend *>(context)
                      ->alloc(detail::as_owned_cpp_obj<MemoryLayout, z_loaned_memory_layout_t>(layout))
                      .take();
}
inline void free_fn(const struct z_chunk_descriptor_t *chunk, void *context) {
    static_cast<CppShmProviderBackend *>(context)->free(*chunk);
}
inline size_t defragment_fn(void *context) { return static_cast<CppShmProviderBackend *>(context)->defragment(); }
inline size_t available_fn(void *context) { return static_cast<CppShmProviderBackend *>(context)->available(); }
inline void layout_for_fn(struct z_owned_memory_layout_t *layout, void *context) {
    static_cast<CppShmProviderBackend *>(context)->layout_for(
        detail::as_cpp_obj_mut<MemoryLayout, z_owned_memory_layout_t>(layout));
}
}
}  // namespace shm::provider_backend::closures

}  // end of namespace zenoh
