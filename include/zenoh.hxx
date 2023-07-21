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

// ZENOHCXX_ZENOHPICO and ZENOHCXX_ZENOHC are mutually exclusive when using this header
#if defined(ZENOHCXX_ZENOHPICO) && defined(ZENOHCXX_ZENOHC)
#error("Only one of ZENOHCXX_ZENOHPICO and ZENOHCXX_ZENOHC should be defined. \
Explictly include zenohpico.hxx and zenohc.hxx to use both libraies in the same program\
under namespaces zenohpico and zenohc respectively.")
#endif
#if !defined(ZENOHCXX_ZENOHPICO) && !defined(ZENOHCXX_ZENOHC)
#error("Either ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC should be defined")
#endif

#if defined(ZENOHCXX_ZENOHPICO)
#include "zenohpico.hxx"
namespace zenoh = zenohpico;
#elif defined(ZENOHCXX_ZENOHC)
#include "zenohc.hxx"
namespace zenoh = zenohc;
#endif
