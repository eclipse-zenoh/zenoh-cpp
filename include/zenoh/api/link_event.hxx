//
// Copyright (c) 2026 ZettaScale Technology
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

#if defined(Z_FEATURE_UNSTABLE_API)

#include "../zenohc.hxx"
#include "base.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "link.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Represents a link connectivity event (link added or removed).
class LinkEvent : public Owned<::z_owned_link_event_t> {
   public:
    /// @name Methods

    /// @brief Get the kind of this event.
    /// @return `SampleKind::Z_SAMPLE_KIND_PUT` for link added, `SampleKind::Z_SAMPLE_KIND_DELETE` for link
    /// removed.
    SampleKind get_kind() const { return ::z_link_event_kind(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the link associated with this event.
    /// @return reference to the `Link`.
    const Link& get_link() const {
        return interop::as_owned_cpp_ref<Link>(::z_link_event_link(interop::as_loaned_c_ptr(*this)));
    }
};

}  // namespace zenoh
#endif
