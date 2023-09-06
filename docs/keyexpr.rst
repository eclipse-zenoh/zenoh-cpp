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

Key Expressions
===============

Classes and functions for processing key expressions. See `zenoh abstractions`_ and `Key Expression RFC`_ for detailed explanation of key expression concept.

Key expression can be registered in the :cpp:class:`zenoh::Session` object with :cpp:func:`zenoh::Session::declare_keyexpr` method. The unique id is internally assinged to the key expression string in this case.

There is a subtle difference between zenoh-c and zenoh-pico implementations of key expressions which affects the key expression API.
In the zenoh-c the key expresssion structure stores both the assigned id and the original key expression string.
On the contrary, for declared key expressions the zenoh-pico keeps only the unique id in the key expression instance. The string
representation is avaliable through the :cpp:func:`zenoh::KeyExprView::resolve` method only, which accesses the :cpp:class:`zenoh::Session` instance.

Therefore in zenoh-pico case the text compare operations between the :cpp:class:`KeyExprView` objects will return error result for declared key expressions.

For that reason the operations without explicit error handling are disabled for zenoh-pico.
The methods which automatically performs the necessary resolving are provided instead: :cpp:func:`zenoh::Session::keyexpr_equals`, :cpp:func:`zenoh::Session::keyexpr_includes`, :cpp:func:`zenoh::Session::keyexpr_intersects`.

.. _`zenoh abstractions`: https://zenoh.io/docs/manual/abstractions/
.. _`Key Expression RFC`: https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md

.. doxygenfunction:: zenoh::keyexpr_canonize(std::string& s, ErrNo& error)

.. doxygenfunction:: zenoh::keyexpr_canonize(std::string& s)

.. doxygenfunction:: zenoh::keyexpr_is_canon(const std::string_view& s, ErrNo& error)

.. doxygenfunction:: zenoh::keyexpr_is_canon(const std::string_view& s)

.. doxygenfunction:: zenoh::keyexpr_concat(const zenoh::KeyExprView& k, const std::string_view& s)

.. doxygenfunction:: zenoh::keyexpr_join(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: zenoh::keyexpr_equals(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: zenoh::keyexpr_equals(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: zenoh::keyexpr_includes(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: zenoh::keyexpr_includes(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: zenoh::keyexpr_intersects(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: zenoh::keyexpr_intersects(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenstruct:: zenoh::KeyExprUnchecked
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::KeyExprView
   :members:
   :membergroups: Constructors Operators Methods
 
.. doxygenclass:: zenoh::KeyExpr
   :members:
   :membergroups: Constructors Operators Methods
