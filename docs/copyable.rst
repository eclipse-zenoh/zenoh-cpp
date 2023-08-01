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

Copyable types
==============

Copyable types are the types which can be freely copied and passed by value.
They either don't reference external resources or they reference resources 
without taking ownership.

All these types are based on `Copyable` template.

.. doxygenstruct:: zenohcxx::Copyable
   :members:

.. doxygenclass:: zenoh::BytesView
   :members:

.. doxygenstruct:: zenoh::StrArrayView

.. doxygenstruct:: zenoh::_StrArrayView
   :members: