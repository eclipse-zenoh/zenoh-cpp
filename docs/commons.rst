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

Commonly used types
===================

Enums
-----

Enum types are C++ - style typedefs for corrresponding enums of `zenoh-c`_ / `zenoh-pico`_ C API.

.. doxygentypedef:: zenoh::SampleKind
.. doxygentypedef:: zenoh::ConsolidationMode

.. doxygentypedef:: zenoh::Reliability

.. doxygentypedef:: zenoh::CongestionControl

.. doxygentypedef:: zenoh::Priority

.. doxygentypedef:: zenoh::QueryTarget

.. doxygentypedef:: zenoh::WhatAmI
    
.. doxygentypedef:: zenoh::Locality

.. doxygenfunction:: whatami_as_str

.. _zenoh-c: https://zenoh-c.readthedocs.io
.. _zenoh-pico: https://zenoh-pico.readthedocs.io

Source Info
-----------

.. doxygenclass:: zenoh::Id
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenfunction:: zenoh::operator<<(std::ostream &os, const Id &id)

.. doxygenclass:: zenoh::EntityGlobalId
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::SourceInfo
   :members:
   :membergroups: Constructors Operators Methods

Timestamp
---------
.. doxygenclass:: zenoh::Timestamp
   :members:
   :membergroups: Constructors Operators Methods


Encoding
--------
.. doxygenclass:: zenoh::Encoding
   :members:
   :membergroups: Constructors Operators Methods

Sample
------
.. doxygenclass:: zenoh::Sample
   :members:
   :membergroups: Constructors Operators Methods

Logging
-------

.. doxygenfunction:: init_logging
