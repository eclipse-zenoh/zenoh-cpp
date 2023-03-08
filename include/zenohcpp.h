//
// Copyright (c) 2022 ZettaScale Technology
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

#if defined(ZENOHCPP_ZENOHC)
#include "zenoh.h"
#elif defined(ZENOHCPP_ZENOHPICO)
#include "zenoh-pico.h"
#else
#error ZENOHCPP_ZENOHC or ZENOHCPP_ZENOHPICO should be defined to select base C zenoh library
#endif

// #include "zenohcpp/zenohcpp_base.h"
// #include "zenohcpp/zenohcpp_channel.h"
// #include "zenohcpp/zenohcpp_objects.h"
#include "zenohcpp/zenohcpp_structs.h"