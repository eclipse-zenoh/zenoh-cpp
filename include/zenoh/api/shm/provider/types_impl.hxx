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

#include "../../interop.hxx"
#include "types.hxx"

namespace zenoh {

struct Converters {
    static inline BufLayoutAllocResult from(z_buf_layout_alloc_result_t& c_result) {
        switch (c_result.status) {
            case zc_buf_layout_alloc_status_t::ZC_BUF_LAYOUT_ALLOC_STATUS_OK:
                return std::move(interop::as_owned_cpp_ref<ZShmMut>(&c_result.buf));
            case zc_buf_layout_alloc_status_t::ZC_BUF_LAYOUT_ALLOC_STATUS_LAYOUT_ERROR:
                return c_result.layout_error;
            case zc_buf_layout_alloc_status_t::ZC_BUF_LAYOUT_ALLOC_STATUS_ALLOC_ERROR:
            default:
                return c_result.alloc_error;
        }
    }

    static inline BufAllocResult from(z_buf_alloc_result_t& c_result) {
        switch (c_result.status) {
            case zc_buf_alloc_status_t::ZC_BUF_ALLOC_STATUS_OK:
                return std::move(interop::as_owned_cpp_ref<ZShmMut>(&c_result.buf));
            case zc_buf_alloc_status_t::ZC_BUF_ALLOC_STATUS_ALLOC_ERROR:
            default:
                return c_result.error;
        }
    }
};

}  // end of namespace zenoh
