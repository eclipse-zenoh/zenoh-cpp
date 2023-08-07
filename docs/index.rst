..
.. Copyright (c) 2023 ZettaScale Technology
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

*********
zenoh-cpp
*********

The *zenoh-cpp* library provides a client C++ API for the zenoh network protocol.

An introduction to Zenoh and its concepts is available on `zenoh.io`_. Since the zenoh-cpp is a header-only wrapper 
library over the `zenoh-c`_ and `zenoh-pico`_ C libraries, it can be useful to reference the documentation of these libraries as well.
The zenoh-c library is C interface to main implementation of zenoh on Rust. So documentation for `zenoh`_ Rust API is also may help.

.. toctree::
    :maxdepth: 10

    examples
    api
    genindex

.. _zenoh.io: https://zenoh.io
.. _zenoh: https://docs.rs/zenoh
.. _zenoh-c: https://zenoh-c.readthedocs.io
.. _zenoh-pico: https://zenoh-pico.readthedocs.io 