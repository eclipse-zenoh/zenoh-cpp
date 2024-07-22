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
inline void result(void* context, struct z_buf_alloc_result_t* result) {
    auto interface = static_cast<AllocLayoutAsyncInterface*>(context);
    interface->on_result(Converters::from(*result));
}

inline void drop(void* context) {
    auto interface = static_cast<AllocLayoutAsyncInterface*>(context);
    delete interface;
}
}
}  // namespace shm::provider::closures

class AllocLayout : public Owned<::z_owned_alloc_layout_t> {
   public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a new Alloc Layout for SHM Provider.
    AllocLayout(const ShmProvider& owner_provider, std::size_t size, AllocAlignment alignment, ZError* err = nullptr)
        : Owned(nullptr) {
        __ZENOH_ERROR_CHECK(::z_alloc_layout_new(&this->_0, owner_provider.loan(), size, alignment), err,
                            "Failed to create SHM Alloc Layout");
    }

    BufAllocResult alloc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc(&result, this->loan());
        return Converters::from(result);
    }

    BufAllocResult alloc_gc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc(&result, this->loan());
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag(&result, this->loan());
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_dealloc() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag_dealloc(&result, this->loan());
        return Converters::from(result);
    }

    BufAllocResult alloc_gc_defrag_blocking() const {
        z_buf_alloc_result_t result;
        ::z_alloc_layout_alloc_gc_defrag_blocking(&result, this->loan());
        return Converters::from(result);
    }

    ZError alloc_gc_defrag_async(std::unique_ptr<AllocLayoutAsyncInterface> receiver) const {
        auto rcv = receiver.release();
        ::zc_threadsafe_context_t context = {rcv, &shm::provider::closures::drop};
        return ::z_alloc_layout_threadsafe_alloc_gc_defrag_async(&rcv->_result, this->loan(), context,
                                                                 shm::provider::closures::result);
    }
};
}  // end of namespace zenoh
