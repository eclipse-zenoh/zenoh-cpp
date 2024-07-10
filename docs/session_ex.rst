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

The zenoh session is created using the :cpp:func:`zenoh::Session::open` function, 
consuming the configuration object :cpp:class:`zenoh::Config`.
Then a string is published on "demo/example/simple" key expression.

.. code-block:: c++

   #include "zenoh.hxx"
   using namespace zenoh;

   int main(int argc, char **argv) {
      try {
         Config config = Config::create_default();
         auto session = Session::open(std::move(config));
         session.put(KeyExpr("demo/example/simple"), Bytes::serialize("Simple!"));
      } catch (ZException e) {
         std::cout << "Received an error :" << e.what() << "\n";
      }
   }