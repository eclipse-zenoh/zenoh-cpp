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

#undef __ZENOHCXX_ZENOHPICO
#define __ZENOHCXX_ZENOHC

#include "zenoh.h"
#include "zenohcxx/base.hxx"

namespace zenohc {
using namespace zenohcxx;
namespace z = zenohc;  // to disambiguate names for code analyzers
// clang-format off
// include order is important
#include "zenohcxx/api.hxx"
#include "zenohcxx/impl.hxx"
// clang-format on
}  // namespace zenohc