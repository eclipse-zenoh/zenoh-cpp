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

Classes implementing key expressions. See `zenoh abstractions`_ and `Key Expression RFC`_ for details.

.. _`zenoh abstractions`: https://zenoh.io/docs/manual/abstractions/
.. _`Key Expression RFC`: https://github.com/eclipse-zenoh/roadmap/blob/main/rfcs/ALL/Key%20Expressions.md

.. doxygenfunction:: keyexpr_canonize(std::string& s, ErrNo& error)

.. doxygenfunction:: keyexpr_canonize(std::string& s)

.. doxygenfunction:: keyexpr_is_canon(const std::string_view& s, ErrNo& error)

.. doxygenfunction:: keyexpr_is_canon(const std::string_view& s)

.. doxygenfunction:: keyexpr_concat(const zenoh::KeyExprView& k, const std::string_view& s)

.. doxygenfunction:: keyexpr_join(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: keyexpr_equals(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: keyexpr_equals(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: keyexpr_includes(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: keyexpr_includes(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenfunction:: keyexpr_intersects(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b, ErrNo& error)

.. doxygenfunction:: keyexpr_intersects(const zenoh::KeyExprView& a, const zenoh::KeyExprView& b)

.. doxygenstruct:: zenoh::KeyExprUnchecked
   :members:
   :membergroups: Constructors Operators Methods

.. doxygenstruct:: zenoh::KeyExprView
   :members:
   :membergroups: Constructors Operators Methods
 
.. doxygenclass:: zenoh::KeyExpr
   :members:
   :membergroups: Constructors Operators Methods
