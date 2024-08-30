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

All failable Zenoh methods accept pointer to zenoh::ZResult as an optional argument.
If it is not provided (or set to ``nullptr``) a ``zenoh::ZException`` will be thrown in case of failure.
Otherwise a error code will be written to provided pointer and no exception will be thrown from Zenoh side.
If corresponding method is expected to return or consume (via ``std::move``) any objects, they will be reset to
gravestone state (i.e. None of the functions or methods will work with the object in this state, except 
explicit conversion to ``bool``, which will return false).

.. doxygenclass:: zenoh::ZException
   :members:
   :membergroups: Constructors Operators Methods
   
.. doxygentypedef:: zenoh::ZResult
