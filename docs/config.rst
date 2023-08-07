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

Configuration
=============

The classes which defines network configuration parameters

Builder functions
-----------------

.. doxygenfunction:: zenoh::config_peer

.. doxygenfunction:: zenoh::config_from_file

.. doxygenfunction:: zenoh::config_client(const zenoh::StrArrayView& peers)

.. doxygenfunction:: zenoh::config_client(const std::initializer_list<const char*>& peers)

Configuration classes
---------------------

.. doxygenclass:: zenoh::Config
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ScoutingConfig
   :members:
   :membergroups: Constructors Operators Methods