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

Closure templates
=================

zenoh-c closure is a structure with 3 fields: context pointer ``context``, callback ``call`` and finalizer ``drop``. Context is a pointer to closure data, 
callback is a function pointer and finalizer is a function pointer which is called when closure is destroyed. Example of zenoh-c closure:

.. code-block:: c++
   
   typedef struct z_owned_closure_query_t {
      void *context;
      void (*call)(const struct z_query_t*, void *context);
      void (*drop)(void*);
   } z_owned_closure_query_t;

zenoh-cpp closures are wrappers around zenoh-c closures. These wrappers allows to construct closures from any callable C++ object, like lambda or function pointer.

All zenoh-cpp closures are based on the one of the following templates: 
:cpp:class:`zenohcxx::ClosureConstPtrParam` and :cpp:class:`zenohcxx::ClosureMoveParam`. First one accepts ``const ZCPP_PARAM&``, second one accepts ``ZCPP_PARAM&&``.

These templates allows to construct closures from any callable C++ object:

- function pointer of type ``void (func*)(const ZCPP_PARAM&)`` or ``void (func*)(ZCPP_PARAM&&)``

   Example:


   .. code-block:: c++

      void on_query(const Query&) { ... }


   .. code-block:: c++

      session.declare_queryable("foo/bar", on_query);

- any object which can be called with corresponding parameter, e.g. lambda or custom object. If object is passed by
  move, closure will take ownership of it, otherwise it will store reference to it.

   Example:

   .. code-block:: c++

      session.declare_queryable("foo/bar", [](const Query&) { ... });

   or

   .. code-block:: c++

      struct OnQuery {
         void operator()(const Query&) { ... }
         ~OnQuery() { ... }
      };

   .. code-block:: c++

      OnQuery on_query;
      session.declare_queryable("foo/bar", std::move(on_query));


.. doxygenclass:: zenohcxx::ClosureConstRefParam
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenclass:: zenohcxx::ClosureMoveParam
   :members:
   :membergroups: Constructors Operators Methods
