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

Extensions
==========
Extra functionality, which is not a part of core Zenoh API.

Serialization / Deserialization
-------------------------------
Support for data serialization and deserialization.

.. doxygenclass:: zenoh::ext::Serializer
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenoh::ext::Deserializer
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenfunction:: zenoh::ext::serialize
.. doxygenfunction:: zenoh::ext::deserialize


Session Extension
-----------------
Extra Zenoh entities.

.. doxygenclass:: zenoh::ext::SessionExt
   :members:
   :membergroups: Constructors Operators Methods Fields

.. doxygenclass:: zenoh::ext::PublicationCache
   :members:
   :membergroups: Constructors Operators Methods Fields

.. doxygenclass:: zenoh::ext::QueryingSubscriber
   :members:
   :membergroups: Constructors Operators Methods Fields

.. doxygenclass:: zenoh::ext::AdvancedPublisher
   :members:
   :membergroups: Constructors Operators Methods Fields

.. doxygenclass:: zenoh::ext::AdvancedSubscriber
   :members:
   :membergroups: Constructors Operators Methods Fields