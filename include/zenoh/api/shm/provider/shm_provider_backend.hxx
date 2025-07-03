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

#include <memory>

#include "../../base.hxx"
#include "../../interop.hxx"
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
    virtual ProtocolId id() const = 0;
    virtual ~CppShmProviderBackendIface() = default;
};

class CppShmProviderBackend : public CppShmProviderBackendIface {};

class CppShmProviderBackendThreadsafe : public CppShmProviderBackend {};

// Ensure that function pointers are defined with extern C linkage
namespace shm::provider_backend::closures {
extern "C" {
inline void _z_cpp_shm_provider_backend_drop_fn(void *context) { delete static_cast<CppShmProviderBackend *>(context); }
inline void _z_cpp_shm_provider_backend_alloc_fn(struct z_owned_chunk_alloc_result_t *out_result,
                                                 const struct z_loaned_memory_layout_t *layout, void *context) {
    *out_result = interop::move_to_c_obj(
        static_cast<CppShmProviderBackend *>(context)->alloc(interop::as_owned_cpp_ref<MemoryLayout>(layout)));
}
inline void _z_cpp_shm_provider_backend_free_fn(const struct z_chunk_descriptor_t *chunk, void *context) {
    static_cast<CppShmProviderBackend *>(context)->free(*chunk);
}
inline size_t _z_cpp_shm_provider_backend_defragment_fn(void *context) {
    return static_cast<CppShmProviderBackend *>(context)->defragment();
}
inline size_t _z_cpp_shm_provider_backend_available_fn(void *context) {
    return static_cast<CppShmProviderBackend *>(context)->available();
}
inline void _z_cpp_shm_provider_backend_layout_for_fn(struct z_owned_memory_layout_t *layout, void *context) {
    static_cast<CppShmProviderBackend *>(context)->layout_for(interop::as_owned_cpp_ref<MemoryLayout>(layout));
}
inline ProtocolId _z_cpp_shm_provider_backend_id_fn(void *context) {
    return static_cast<CppShmProviderBackend *>(context)->id();
}
}
}  // namespace shm::provider_backend::closures

}  // end of namespace zenoh
