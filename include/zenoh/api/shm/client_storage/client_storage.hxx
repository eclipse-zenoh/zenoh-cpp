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

#include <type_traits>

#include "../../base.hxx"
#include "../../interop.hxx"
#include "../client/shm_client.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A storage for SHM clients. Session constructed with instance of this type
/// gets capabilities to read SHM buffers for Protocols added to this instance.
class ShmClientStorage : public Owned<::z_owned_shm_client_storage_t> {
    using Owned::Owned;

   public:
    /// @name Constructors

    /// @brief Create ShmClientStorage referencing a default global list of SHM clients
    static ShmClientStorage global() {
        ShmClientStorage storage(nullptr);
        z_ref_shm_client_storage_global(&storage._0);
        return storage;
    }

    /// @brief Creates ShmClient storage with zenoh-default set of SHM clients
    ShmClientStorage() : Owned(nullptr) { z_shm_client_storage_new_default(&this->_0); }

    /// @brief Create ShmClientStorage from a list of SHM clients
    /// @param container arbitrary container of SHM clients. ShmClientStorage takes the ownership of the clients.
    /// @param add_default_client_set if true, the resulting ShmClientStorage will also include zenoh-default set of SHM
    /// clients
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class Container,
              typename _T = std::enable_if<std::is_same<
                  typename std::iterator_traits<typename Container::iterator>::value_type, ShmClient>::value>>
    ShmClientStorage(Container&& container, bool add_default_client_set, ZResult* err = nullptr)
        : ShmClientStorage(std::make_move_iterator(container.begin()), std::make_move_iterator(container.end()),
                           add_default_client_set, err) {}

    /// @brief Create ShmClientStorage from a range of SHM clients
    /// @param begin start iterator of SHM clients. ShmClientStorage takes the ownership of the clients.
    /// @param end end iterator of SHM clients.
    /// @param add_default_client_set if true, the resulting ShmClientStorage will also include zenoh-default set of SHM
    /// clients
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class I, typename _T =
                           std::enable_if<std::is_same<typename std::iterator_traits<I>::value_type, ShmClient>::value>>
    ShmClientStorage(std::move_iterator<I> begin, std::move_iterator<I> end, bool add_default_client_set,
                     ZResult* err = nullptr)
        : Owned(nullptr) {
        // create client list
        ClientList list;

        // fill list with clients
        for (std::move_iterator<I> it = begin; it != end; ++it) {
            __ZENOH_RESULT_CHECK(zc_shm_client_list_add_client(interop::as_loaned_c_ptr(list), z_move(it->_0)), err,
                                 "Failed to form list of SHM clients");
        }

        // create client storage from the list
        __ZENOH_RESULT_CHECK(
            z_shm_client_storage_new(&this->_0, interop::as_loaned_c_ptr(list), add_default_client_set), err,
            "Failed to create SHM client storage!");
    }

    /// @brief Performs a shallow copy of ShmClientStorage
    ShmClientStorage(const ShmClientStorage& other) : Owned(nullptr) {
        z_shm_client_storage_clone(&this->_0, interop::as_loaned_c_ptr(other));
    }

   private:
    class ClientList : public Owned<zc_owned_shm_client_list_t> {
        friend class ShmClientStorage;

       public:
        ClientList() : Owned(nullptr) { zc_shm_client_list_new(&this->_0); }
    };
};

}  // end of namespace zenoh
