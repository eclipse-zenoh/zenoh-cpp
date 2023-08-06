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

Headers and namespaces
======================
.. index:: zenohc:: namespace
.. index:: zenohpico:: namespace

Zenoh C++ API is available through one of the following headers: "zenohc.hxx", "zenohpico.hxx" and "zenoh.hxx".
The first two defines C++ wrappers for C libraries `zenoh-c`_ and `zenoh-pico`_ respectively in namespaces ``zenohc`` and ``zenohpico``:

.. code-block:: cpp

   #include "zenohc.hxx"
   using namespace zenohc;

.. code-block:: cpp

   #include "zenohpico.hxx"
   using namespace zenohpico;

.. index:: zenoh: namespace

The header "zenoh.hxx" selects between one of headers above depending on the macro ``ZENOHCXX_ZENOHC`` or ``ZENOHCXX_ZENOHPICO``. It also aliases
the corrresponding namespace to ``zenoh``:

.. code-block:: cpp

   #define ZENOHCXX_ZENOHC   
   // #define ZENOHCXX_ZENOHPICO
   #include "zenoh.hxx"
   using namespace zenoh;

This can be useful if it's necessary to compile the same source for both `zenoh-c`_ and `zenoh-pico`_.

.. index:: z:: namespace

There is also ``z::`` namespace which appears in the method and function prototypes in source files. 
It is an internal alias for the current namespace (``zenohc`` or ``zenohpico``), 
it's never used in the user code.

In the documentation below, we will use the ``zenoh`` namespace.

.. _zenoh-c: https://zenoh-c.readthedocs.io
.. _zenoh-pico: https://zenoh-pico.readthedocs.io 