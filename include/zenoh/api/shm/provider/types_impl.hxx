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

#include "types.hxx"

namespace zenoh {

struct Converters {
    static inline BufLayoutAllocResult from(z_buf_layout_alloc_result_t& c_result) {
        if (z_check(c_result.buf)) {
            return ZShmMut(&c_result.buf);
        } else if (c_result.error_is_alloc) {
            return c_result.alloc_error;
        } else {
            return c_result.layout_error;
        }
    }

    static inline BufAllocResult from(z_buf_alloc_result_t& c_result) {
        if (z_check(c_result.buf)) {
            return ZShmMut(&c_result.buf);
        } else {
            return c_result.error;
        }
    }
};

}  // end of namespace zenoh
