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

#pragma once

#define __ZENOHCXX_ZENOHPICO
#undef __ZENOHCXX_ZENOHC

#include "zenoh-pico.h"
#include "zenohcxx/includes.hxx"

namespace zenohpico {
// #include "zenohcpp/zenohcpp_base.h"
// #include "zenohcpp/zenohcpp_channel.h"
// #include "zenohcpp/zenohcpp_objects.h"
#include "zenohcxx/base.hxx"
#include "zenohcxx/structs.hxx"
}