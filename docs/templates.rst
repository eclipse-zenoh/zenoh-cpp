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

Base templates
==============

There are two kinds of classes in zenoh-cpp: :cpp:class:`zenohcxx::Copyable`
and :cpp:class:`zenohcxx::Owned`.

Copyable types are the types which can be freely copied and passed by value.
They either don't reference external resources or they reference resources 
without taking ownership.

Owned types are the types which own resources. They implements move semantics and
cannot be copied. They releases their resources when they are destroyed.

This separation follows the logic of the `zenoh-c`_ and `zenoh-pico`_ API. Owned zenoh-cpp types are the wrappers
for the corresponding ``::z_owned_XXX_t`` types. Copyable zenoh-cpp types are the wrappers for the
non-owned ``::z_XXX_t`` types.

.. doxygenstruct:: zenohcxx::Copyable
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenohcxx::Owned
   :members:
   :membergroups: Constructors Operators Methods

.. _zenoh-c: https://zenoh-c.readthedocs.io
.. _zenoh-pico: https://zenoh-pico.readthedocs.io 