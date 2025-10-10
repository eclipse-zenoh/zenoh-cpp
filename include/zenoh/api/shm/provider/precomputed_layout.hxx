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
#include "types.hxx"
#include "types_impl.hxx"

namespace zenoh {

class ShmProvider;

struct PrecomputedLayoutAsyncInterface {
    z_buf_alloc_result_t _result;
    virtual void on_result(BufAllocResult&& result) = 0;
    virtual ~PrecomputedLayoutAsyncInterface() = default;
};

// Ensure that function pointers are defined with extern C linkage
namespace shm::provider::closures {
extern "C" {
inline void _z_precomputed_layout_async_interface_result_fn(void* context, struct z_buf_alloc_result_t* result) {
    auto interface = static_cast<PrecomputedLayoutAsyncInterface*>(context);
    interface->on_result(Converters::from(*result));
}

inline void _z_precomputed_layout_async_interface_drop_fn(void* context) {
    auto interface = static_cast<PrecomputedLayoutAsyncInterface*>(context);
    delete interface;
}
}
}  // namespace shm::provider::closures

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
class PrecomputedLayout : public Owned<::z_owned_precomputed_layout_t> {
    friend class ShmProvider;

   protected:
    PrecomputedLayout(zenoh::detail::null_object_t) : Owned(nullptr) {}

   public:
    /// @name Constructors

    /// @warning This API has been marked as deprecated, use `ShmProvider::alloc_layout` instead.
    PrecomputedLayout(const ShmProvider& owner_provider, std::size_t size, ZResult* err = nullptr);

    /// @warning This API has been marked as deprecated, use `ShmProvider::alloc_layout` instead.
    PrecomputedLayout(const ShmProvider& owner_provider, std::size_t size, AllocAlignment alignment,
                      ZResult* err = nullptr);

    /// @name Methods
    BufAllocResult alloc() const {
        z_buf_alloc_result_t result;
        ::z_precomputed_layout_alloc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc() const {
        z_buf_alloc_result_t result;
        ::z_precomputed_layout_alloc_gc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag() const {
        z_buf_alloc_result_t result;
        ::z_precomputed_layout_alloc_gc_defrag(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_dealloc() const {
        z_buf_alloc_result_t result;
        ::z_precomputed_layout_alloc_gc_defrag_dealloc(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_blocking() const {
        z_buf_alloc_result_t result;
        ::z_precomputed_layout_alloc_gc_defrag_blocking(&result, interop::as_loaned_c_ptr(*this));
        return Converters::from(result);
    }

    ZResult alloc_gc_defrag_async(std::unique_ptr<PrecomputedLayoutAsyncInterface> receiver) const {
        auto rcv = receiver.release();
        ::zc_threadsafe_context_t context = {{rcv},
                                             &shm::provider::closures::_z_precomputed_layout_async_interface_drop_fn};
        return ::z_precomputed_layout_threadsafe_alloc_gc_defrag_async(
            &rcv->_result, interop::as_loaned_c_ptr(*this), context,
            shm::provider::closures::_z_precomputed_layout_async_interface_result_fn);
    }
};

/// @warning This API has been marked as deprecated, use `PrecomputedLayout` instead.
using AllocLayout = PrecomputedLayout;

}  // end of namespace zenoh
