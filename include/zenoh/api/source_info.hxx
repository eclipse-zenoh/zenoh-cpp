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

#include "../zenohc.hxx"
#include "base.hxx"
#include "id.hxx"
#include "interop.hxx"

namespace zenoh {
/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief The global unique id of a Zenoh entity.
class EntityGlobalId : public Copyable<::z_entity_global_id_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// Get Zenoh id.
    Id id() const { return interop::into_copyable_cpp_obj<Id>(::z_entity_global_id_zid(&this->inner())); }

    /// Get eid.
    uint32_t eid() const { return ::z_entity_global_id_eid(&this->inner()); }

    /// @name Operators

    /// @brief Equality relation.
    /// @param other an entity global id to compare with.
    /// @return ``true`` if both ids are equal, ``false`` otherwise.
    bool operator==(const EntityGlobalId& other) const {
        return this->eid() == other.eid() && this->id() == other.id();
    };
};

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Informations on the Zenoh source.
class SourceInfo : public Copyable<::z_source_info_t> {
    using Copyable::Copyable;
    friend struct interop::detail::Converter;

   public:
    /// @name Constructors

    /// @brief Construct from global id and sequence number.
    SourceInfo(const EntityGlobalId& id, uint32_t sn)
        : Copyable(::z_source_info_new(interop::as_copyable_c_ptr(id), sn)) {}

    /// @name Methods

    /// @brief Get the source id.
    EntityGlobalId id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(::z_source_info_id(&this->inner()));
    }

    /// @brief Get the sequence number of the sample from the given source.
    uint32_t sn() const { return ::z_source_info_sn(&this->inner()); }
};
}  // namespace zenoh
