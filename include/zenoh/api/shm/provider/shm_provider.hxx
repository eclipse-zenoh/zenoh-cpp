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
#include "../common/common.hxx"
#include "chunk.hxx"
#include "shm_provider_backend.hxx"
#include "types.hxx"
#include "types_impl.hxx"

namespace zenoh {

class ShmProviderAsyncInterface {
    friend class ShmProvider;

    z_buf_layout_alloc_result_t _result;

    virtual void on_result(BufLayoutAllocResult&& result) = 0;

    static void drop(void* context) {
        auto interface = static_cast<ShmProviderAsyncInterface*>(context);
        delete interface;
    }

    static void result(void* context, struct z_buf_layout_alloc_result_t* result) {
        auto interface = static_cast<ShmProviderAsyncInterface*>(context);
        interface->on_result(Converters::from(*result));
    }

   public:
    virtual ~ShmProviderAsyncInterface() = default;
};

class ShmProvider : public Owned<::z_owned_shm_provider_t> {
    friend class AllocLayout;

   protected:
    ShmProvider(zenoh::detail::null_object_t) : Owned(nullptr) {}

   public:
    BufLayoutAllocResult alloc(size_t size) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc(&result, interop::as_loaned_c_ptr(*this), size);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc(size_t size, AllocAlignment alignment) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_aligned(&result, interop::as_loaned_c_ptr(*this), size, alignment);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc(size_t size) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc(&result, interop::as_loaned_c_ptr(*this), size);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc(size_t size, AllocAlignment alignment) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_aligned(&result, interop::as_loaned_c_ptr(*this), size, alignment);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag(size_t size) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag(&result, interop::as_loaned_c_ptr(*this), size);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag(size_t size, AllocAlignment alignment) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag_aligned(&result, interop::as_loaned_c_ptr(*this), size, alignment);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag_dealloc(size_t size) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag_dealloc(&result, interop::as_loaned_c_ptr(*this), size);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag_dealloc(size_t size, AllocAlignment alignment) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag_dealloc_aligned(&result, interop::as_loaned_c_ptr(*this), size, alignment);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag_blocking(size_t size) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag_blocking(&result, interop::as_loaned_c_ptr(*this), size);
        return Converters::from(result);
    }

    BufLayoutAllocResult alloc_gc_defrag_blocking(size_t size, AllocAlignment alignment) const {
        z_buf_layout_alloc_result_t result;
        ::z_shm_provider_alloc_gc_defrag_blocking_aligned(&result, interop::as_loaned_c_ptr(*this), size, alignment);
        return Converters::from(result);
    }

    ZResult alloc_gc_defrag_async(size_t size, std::unique_ptr<ShmProviderAsyncInterface> receiver) const {
        auto rcv = receiver.release();
        ::zc_threadsafe_context_t context = {{rcv}, &ShmProviderAsyncInterface::drop};
        return ::z_shm_provider_alloc_gc_defrag_async(&rcv->_result, interop::as_loaned_c_ptr(*this), size, context,
                                                      ShmProviderAsyncInterface::result);
    }

    ZResult alloc_gc_defrag_async(size_t size, AllocAlignment alignment,
                                  std::unique_ptr<ShmProviderAsyncInterface> receiver) const {
        auto rcv = receiver.release();
        ::zc_threadsafe_context_t context = {{rcv}, &ShmProviderAsyncInterface::drop};
        return ::z_shm_provider_alloc_gc_defrag_aligned_async(&rcv->_result, interop::as_loaned_c_ptr(*this), size,
                                                              alignment, context, ShmProviderAsyncInterface::result);
    }

    void defragment() const { ::z_shm_provider_defragment(interop::as_loaned_c_ptr(*this)); }

    std::size_t garbage_collect() const { return ::z_shm_provider_garbage_collect(interop::as_loaned_c_ptr(*this)); }

    std::size_t available() const { return ::z_shm_provider_available(interop::as_loaned_c_ptr(*this)); }

    ZShmMut map(AllocatedChunk&& chunk, std::size_t len) const {
        z_owned_shm_mut_t result;
        ::z_shm_provider_map(&result, interop::as_loaned_c_ptr(*this), chunk, len);
        return std::move(interop::as_owned_cpp_ref<ZShmMut>(&result));
    }
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
class CppShmProvider : public ShmProvider {
    friend class AllocLayout;

   public:
    using ShmProvider::ShmProvider;

    /// @name Constructors

    /// @brief Create a new CPP-defined ShmProvider.
    CppShmProvider(std::unique_ptr<CppShmProviderBackend> backend) : ShmProvider(zenoh::detail::null_object) {
        // init context
        zc_context_t context = {backend.release(),
                                &shm::provider_backend::closures::_z_cpp_shm_provider_backend_drop_fn};

        // init callbacks
        zc_shm_provider_backend_callbacks_t callbacks = {
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_alloc_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_free_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_defragment_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_available_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_layout_for_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_id_fn,
        };

        // create provider
        ::z_shm_provider_new(&this->_0, context, callbacks);
    }

    /// @brief Create a new CPP-defined threadsafe ShmProvider.
    CppShmProvider(std::unique_ptr<CppShmProviderBackendThreadsafe> backend) : ShmProvider(zenoh::detail::null_object) {
        // init context
        ::zc_threadsafe_context_t context = {{backend.release()},
                                             &shm::provider_backend::closures::_z_cpp_shm_provider_backend_drop_fn};

        // init callbacks
        ::zc_shm_provider_backend_callbacks_t callbacks = {
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_alloc_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_free_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_defragment_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_available_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_layout_for_fn,
            &shm::provider_backend::closures::_z_cpp_shm_provider_backend_id_fn,
        };

        // create provider
        ::z_shm_provider_threadsafe_new(&this->_0, context, callbacks);
    }
};

}  // end of namespace zenoh
