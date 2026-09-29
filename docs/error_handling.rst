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

Error Handling
==============

All failable Zenoh methods accept a pointer to ``zenoh::ZResult``.

If the pointer is not ``nullptr``, the resulting error code is stored in the location it points to.
If the pointer is ``nullptr`` and exceptions are enabled (which is usually the case), a ``zenoh::ZException`` is thrown on failure.
If the pointer is ``nullptr`` and exceptions are disabled, ``abort()`` is called on failure.

Therefore, when exceptions are unavailable, you must always provide a valid ``zenoh::ZResult`` pointer.

If corresponding method is expected to return or consume (via ``std::move``) any objects, they will be reset to
gravestone state (i.e. None of the functions or methods will work with the object in this state, except 
explicit conversion to ``bool``, which will return false).

Callbacks
---------

User callbacks, including drop callbacks and custom shared-memory implementations,
should handle exceptions internally. If an exception escapes into a C++ callback
adapter, ``std::terminate()`` is called. The adapters are ``noexcept`` so that
exceptions cannot unwind through the underlying C or Rust runtime. This also
applies to exceptions raised while converting callback arguments or results.

.. doxygenclass:: zenoh::ZException
   :members:
   :membergroups: Constructors Operators Methods
   
.. doxygentypedef:: zenoh::ZResult
