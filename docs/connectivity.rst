..
.. Copyright (c) 2026 ZettaScale Technology
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

Connectivity
============

.. warning:: This API has been marked as unstable: it works as advertised, but it may be changed in a future release.

Classes for monitoring connectivity events at the transport and link levels.
Transport events notify about connections being opened or closed to remote zenoh nodes.
Link events notify about individual data links being added or removed within a transport.

Current transports and links can be retrieved via :cpp:func:`Session::get_transports` and
:cpp:func:`Session::get_links`.

Transport events listeners are declared via :cpp:func:`Session::declare_transport_events_listener`.
Link events listeners are declared via :cpp:func:`Session::declare_link_events_listener`.

Both listener types accept an options struct with a ``history`` field. When ``history`` is set to ``true``,
the listener will immediately receive events for transports or links that were already established
before the listener was declared, in addition to future events.

Transport
---------

.. doxygenclass:: zenoh::Transport
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::TransportEvent
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::TransportEventsListener
   :members:
   :membergroups: Constructors Operators Methods

Link
----

.. doxygenclass:: zenoh::Link
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::LinkEvent
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::LinkEventsListener
   :members:
   :membergroups: Constructors Operators Methods
