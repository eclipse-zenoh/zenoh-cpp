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

#include <functional>
#include <optional>

#include "../../base.hxx"
#include "../../interop.hxx"
#include "zshmmut.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief An immutable SHM buffer
class ZShm : public Owned<::z_owned_shm_t> {
    friend class ZShmMut;
    ZShm(zenoh::detail::null_object_t) : Owned(nullptr) {}
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Create a new ZShm from ZShmMut.
    /// @param mut mutable buffer.
    ZShm(ZShmMut&& mut) : Owned(nullptr) { ::z_shm_from_mut(&this->_0, ::z_move(mut._0)); }

    /// @brief Create a new ZShm from ZShm by performing a shallow SHM reference copy.
    /// @param other ZShm to copy
    ZShm(const ZShm& other) : Owned(nullptr) { ::z_shm_clone(&this->_0, interop::as_loaned_c_ptr(other)); }

    /// @name Methods

    /// @brief Get buffer's const data. It is completely unsafe to to modify SHM data without using ZShmMut interface.
    /// @return pointer to the underlying data.
    const uint8_t* data() const { return ::z_shm_data(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get buffer's data size.
    /// @return underlying data size.
    std::size_t len() const { return ::z_shm_len(interop::as_loaned_c_ptr(*this)); }

    /// @brief Create a new ZShmMut from ZShm.
    /// @param immut immutable buffer, NOTE: the value will not be moved if nullopt returned.
    /// @return mutable buffer or empty option if buffer mutation is impossible.
    static std::optional<ZShmMut> try_mutate(ZShm&& immut) {
        z_owned_shm_mut_t mut_inner;
        if (Z_OK == ::z_shm_mut_try_from_immut(&mut_inner, z_move(immut._0), &immut._0)) {
            return std::move(interop::as_owned_cpp_ref<ZShmMut>(&mut_inner));
        }
        return std::nullopt;
    }

    /// @brief Create a new ZShmMut& from ZShm&.
    /// @param immut immutable buffer, NOTE: the value will not be moved if nullopt returned.
    /// @return mutable buffer or empty option if buffer mutation is impossible.
    static std::optional<std::reference_wrapper<ZShmMut>> try_mutate(ZShm& immut) {
        if (z_loaned_shm_mut_t* shm_mut = ::z_shm_try_reloan_mut(z_loan_mut(immut._0))) {
            return std::ref(interop::as_owned_cpp_ref<ZShmMut>(shm_mut));
        }
        return std::nullopt;
    }
};

}  // end of namespace zenoh
