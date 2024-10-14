..
.. Copyright (c) 2024 ZettaScale Technology
..
.. This program and the accompanying materials are made available under the
.. terms of the Eclipse Public License 2.0 which is available at
.. http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
.. which is available at https://www.apache.org/licenses/LICENSE-2.0.
..
.. SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
..
.. Contributors:
..   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
..

Interoperability with zenoh-c / zenoh-pico
==========================================
This is the list of the functions that can be use for interoperability between zenoh-c/zenoh-pico and
zenoh-c++. These functions essentially perform conversion of c-structs into c++ classes and back. They should be used with
care, and it is up to the user to ensure that all necessary invariants uphold.

.. doxygennamespace:: zenoh::interop
    :content-only:
