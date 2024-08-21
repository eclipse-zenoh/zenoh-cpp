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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef ZENOHCXX_ZENOHPICO
#include "zenoh-pico.h"
#endif
#ifdef ZENOHCXX_ZENOHC
#include "zenoh.h"
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

#include "zenoh.hxx"

using namespace zenoh;

int main() { return 0; }