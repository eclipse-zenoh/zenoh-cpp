#pragma once
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

#if (__cplusplus < 201703L) && (!defined(_MSVC_LANG) || (_MSVC_LANG < 201703L))
#error zenoh-cpp requires a C++17-compliant compiler
#endif

#include "zenoh/api.hxx"
