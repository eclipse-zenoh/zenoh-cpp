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

Enums
=====

Enum types are C++ - style typedefs for corrresponding enums of `zenoh-c`_ / `zenoh-pico`_ C API.

.. doxygentypedef:: ErrorMessage

.. doxygenfunction:: expect

.. doxygentypedef:: ErrNo

.. doxygentypedef:: SampleKind

.. doxygentypedef:: EncodingPrefix

.. doxygentypedef:: ConsolidationMode

.. doxygentypedef:: Reliability

.. doxygentypedef:: CongestionControl

.. doxygentypedef:: Priority

.. doxygentypedef:: QueryTarget

.. doxygenfunction:: query_target_default

.. doxygentypedef:: zenohpico::WhatAmI

.. doxygenenum:: zenohc::WhatAmI

.. doxygenfunction:: as_cstr

.. _zenoh-c: https://zenoh-c.readthedocs.io
.. _zenoh-pico: https://zenoh-pico.readthedocs.io 