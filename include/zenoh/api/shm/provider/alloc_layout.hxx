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

struct AllocLayoutAsyncInterface {
    z_buf_alloc_result_t _result;
    virtual void on_result(BufAllocResult&& result) = 0;
    virtual ~AllocLayoutAsyncInterface() = default;
};

// Ensure that function pointers are defined with extern C linkage
namespace shm::provider::closures {
extern "C" {
inline void _z_alloc_layout_async_interface_result_fn(void* context, struct z_buf_alloc_result_t* result) {
    auto interface = static_cast<AllocLayoutAsyncInterface*>(context);
    interface->on_result(Converters::from(*result));
}

inline void _z_alloc_layout_async_interface_drop_fn(void* context) {
    auto interface = static_cast<AllocLayoutAsyncInterface*>(context);
    delete interface;
}
}
}  // namespace shm::provider::closures

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
class AllocLayout : public Owned<::z_owned_alloc_layout_t> {
   public:
    /// @name Constructors

    /// @brief Create a new Alloc Layout for SHM Provider.
    AllocLayout(const ShmProvider& owner_provider, std::size_t size, ZResult* err = nullptr) : Owned(nullptr) {
        __ZENOH_RESULT_CHECK(::z_alloc_layout_new(&this->_0, interop::as_loaned_c_ptr(owner_provider), size), err,
                             "Failed to create SHM Alloc Layout");
    }

    /// @brief Create a new Alloc Layout for SHM Provider.
    AllocLayout(const ShmProvider& owner_provider, std::size_t size, AllocAlignment alignment, ZResult* err = nullptr)
        : Owned(nullptr) {
        __ZENOH_RESULT_CHECK(
            ::z_alloc_layout_with_alignment_new(&this->_0, interop::as_loaned_c_ptr(owner_provider), size, alignment),
            err, "Failed to create SHM Alloc Layout");
    }

    /// @name Methods
    BufAllocResult alloc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_dealloc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag_dealloc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_blocking() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag_blocking(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    ZResult alloc_gc_defrag_async(std::unique_ptr<AllocLayoutAsyncInterface> receiver) const {
        auto rcv = receiver.release();
        ::zc_threadsafe_context_t context = {{rcv}, &shm::provider::closures::_z_alloc_layout_async_interface_drop_fn};
        return ::z_alloc_layout_threadsafe_alloc_gc_defrag_async(
            &rcv->_result, interop::as_loaned_c_ptr(*this), context,
            shm::provider::closures::_z_alloc_layout_async_interface_result_fn);
    }
};
}  // end of namespace zenoh
