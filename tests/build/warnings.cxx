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

// Disable 'old-style-cast` warning for C headers only
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#ifdef ZENOHCXX_ZENOHPICO
#include "zenoh-pico.h"
#endif
#ifdef ZENOHCXX_ZENOHC
#include "zenoh.h"
#endif
#pragma GCC diagnostic pop

#include "zenoh.hxx"

using namespace zenoh;

int main() { return 0; }