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

#include <variant>

#include "../../interop.hxx"
#include "../buffer/zshmmut.hxx"

namespace zenoh {

/**
 * Allocation errors
 *
 *     - **NEED_DEFRAGMENT**: defragmentation needed
 *     - **OUT_OF_MEMORY**: the provider is out of memory
 *     - **OTHER**: other error
 */
typedef ::z_alloc_error_t AllocError;

/**
 * Layouting errors
 *
 * Z_LAYOUT_ERROR_INCORRECT_LAYOUT_ARGS: layout arguments are incorrect
 * Z_LAYOUT_ERROR_PROVIDER_INCOMPATIBLE_LAYOUT: layout incompatible with provider
 */
typedef ::z_layout_error_t LayoutError;

/// An AllocAlignment.
typedef ::z_alloc_alignment_t AllocAlignment;

class MemoryLayout : public Owned<::z_owned_memory_layout_t> {
    friend class PosixShmProvider;
    MemoryLayout() : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Create a new MemoryLayout.
    /// @param size layout size
    /// @param alignment layout alignment
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    MemoryLayout(size_t size, AllocAlignment alignment, ZResult* err = nullptr) : Owned(nullptr) {
        __ZENOH_RESULT_CHECK(::z_memory_layout_new(&this->_0, size, alignment), err,
                             "Failed to create MemoryLayout: incorrect arguments!");
    }

    size_t size() const {
        size_t size;
        AllocAlignment alignment;
        z_memory_layout_get_data(&size, &alignment, interop::as_loaned_c_ptr(*this));
        return size;
    }

    AllocAlignment alignment() const {
        size_t size;
        AllocAlignment alignment;
        z_memory_layout_get_data(&size, &alignment, interop::as_loaned_c_ptr(*this));
        return alignment;
    }
};

/// SHM chunk allocation result
class ChunkAllocResult : public Owned<::z_owned_chunk_alloc_result_t> {
    friend class CppShmProviderBackend;

   public:
    /// @name Constructors

    /// @brief Create a new ChunkAllocResult that carries successfuly allocated chunk.
    /// @param chunk allocated chunk
    ChunkAllocResult(AllocatedChunk chunk) : Owned(nullptr) { ::z_chunk_alloc_result_new_ok(&this->_0, chunk); }

    /// @brief Create a new ChunkAllocResult that carries error.
    /// @param error allocation error
    ChunkAllocResult(AllocError error) : Owned(nullptr) { ::z_chunk_alloc_result_new_error(&this->_0, error); }
};

/// SHM buffer allocation result
typedef std::variant<ZShmMut, AllocError> BufAllocResult;

/// SHM buffer layouting and allocation result
typedef std::variant<ZShmMut, AllocError, LayoutError> BufLayoutAllocResult;

}  // end of namespace zenoh
