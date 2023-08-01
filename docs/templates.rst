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

Templates
=========

There are two kinds of classes in zenoh-cpp: Copyable and Owned.

Copyable types are the types which can be freely copied and passed by value.
They either don't reference external resources or they reference resources 
without taking ownership.

Owned types are the types which own resources. They implements move semantics and
cannot be copied. They releases their resources when they are destroyed.

This separation follows the logic of the zenoh-c API. Owned zenoh-cpp types are the wrappers
for the corresponding `::z_owned_X_t` types. Copyable zenoh-cpp types are the wrappers for the
non-owned `::z_X_t` types.


.. doxygenstruct:: zenohcxx::Copyable
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenohcxx::Owned
   :members:
   :membergroups: Constructors Operators Methods
