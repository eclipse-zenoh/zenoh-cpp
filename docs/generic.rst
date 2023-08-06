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

Generic types
=============

The classes representing data buffers, data encoding, timestamp, etc. 
There types are frequently used by other parts of the API.

.. doxygenclass:: zenoh::BytesView
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::Str
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::StrArrayView

.. doxygenstruct:: zenoh::_StrArrayView
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::Encoding
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::Id
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenfunction:: operator<<(std::ostream& os, const zenoh::Id& id)

.. doxygenstruct:: zenoh::Timestamp
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::Value
   :members:
   :membergroups: Constructors Operators Methods
