//
// Copyright (c) 2024 ZettaScale Technology
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

#pragma once

#include "../detail/interop.hxx"
#include "base.hxx"
#include "../zenohc.hxx"
#include "id.hxx"


namespace zenoh {

class EntityGlobalId : public Copyable<::z_entity_global_id_t> {
    using Copyable::Copyable;

    /// @name Constructors

    /// @brief Create new entity global id.
    EntityGlobalId(const Id& id, uint32_t eid) 
        :Copyable({}) {
        ::z_entity_global_id_new(&this->inner(), detail::as_copyable_c_ptr(id), eid);
    }

    /// @name Methods

    /// Return Zenoh id.
    Id id() const { return Id(::z_entity_global_id_zid(&this->inner())); }

    /// Return eid.
    uint32_t eid() const { return ::z_entity_global_id_eid(&this->inner()); }
};


/// @brief Informations on the Zenoh source.
class SourceInfo: public Owned<::z_owned_source_info_t> {
public:
    /// @name Constructors
    using Owned::Owned;

    /// @brief Construct from global id and sequence number.
    SourceInfo(const EntityGlobalId& id, uint64_t sn)
        :Owned(nullptr) { 
        ::z_source_info_new(&this->_0, detail::as_copyable_c_ptr(id), sn); 
    }
    

    /// @name Methods

    /// @brief Return the source id.
    EntityGlobalId id() const { return EntityGlobalId(::z_source_info_id(this->loan()));}

    /// @brief Return the sequence number of the sample from the given source.
    uint64_t sn() const { return ::z_source_info_sn(this->loan());}
};

}
