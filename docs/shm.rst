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

Shared memory support
=====================

The classes for tranferring the data through shared memory if the receiver and transmitter are on the same host

.. doxygenclass:: zenoh::Payload   
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::Shmbuf
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ShmManager
   :members:
   :membergroups: Constructors Operators Methods
