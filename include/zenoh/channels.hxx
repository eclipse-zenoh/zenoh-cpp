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

//
// This file contains structures and classes API without implementations
//

#pragma once

#include "base.hxx"

namespace zenoh::channels {

class FifoChannel {
    size_t _capacity;
public:
    FifoChannel(size_t capacity)
        :_capacity(capacity)
    {}

    template<class T>
    auto into_cb_handler() const {

    }
};

class RingChannel {
    size_t _capacity;
public:
    RingChannel(size_t capacity)
        :_capacity(capacity)
    {}
};

}