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

Session
=======

The session is the main zenoh object. The instance of the session reprsents a single zenoh node in the network.

Builder function
----------------

.. doxygenfunction:: zenoh::open

Utility functions
-----------------

These functions allows to search other zenoh nodes in the network

.. doxygenfunction:: scout(z::ScoutingConfig&& config, z::ClosureHello&& callback, ErrNo& error)

.. doxygenfunction:: scout(z::ScoutingConfig&& config, z::ClosureHello&& callback)


Session class
-------------

.. doxygenclass:: zenoh::Session
   :members:
   :membergroups: Constructors Operators Methods