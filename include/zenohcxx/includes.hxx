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

// Do not put '#pragma once' as this file purposedly included multiple times

// Validate that __ZENOHCXX_ZENOHPICO and __ZENOHCXX_ZENOHC are mutually exclusive
#if defined(__ZENOHCXX_ZENOHPICO) and defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: both __ZENOHCXX_ZENOHC and __ZENOHCXX_ZENOHPICO defined")
#endif

#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "assert.h"
#include "string.h"