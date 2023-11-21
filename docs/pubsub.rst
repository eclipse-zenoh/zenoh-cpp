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

Publish / Subscribe
===================

The publish / subscribe pattern is implemented with classes :cpp:class:`zenoh::Publisher` 
and :cpp:class:`zenoh::Subscriber`.

Publisher example:

.. code-block:: c++

   #include "zenoh.hxx"
   using namespace zenoh;

   int main(int argc, char **argv) {
      Config config;
      auto session = expect<Session>(open(std::move(config)));
      // Publish without creating a Publisher object
      session.put("demo/example/simple", "Simple!");

      // Publish from a Publisher object
      auto publisher = expect<Publisher>(session.declare_publisher("demo/example/simple"));
      publisher.put("Simple!");
   }

Subscriber example:

.. code-block:: c++

   #include "zenoh.hxx"
   #include <iostream>
   using namespace zenoh;

   int main(int argc, char **argv) {
      Config config;
      auto session = expect<Session>(open(std::move(config)));
      auto subscriber = expect<Subscriber>(
         session.declare_subscriber("demo/example/simple", [](const Sample& sample) {
            std::cout << "Received: " << sample.get_payload().as_string_view() << std::endl;
         })
      );
      // Wait for a key press to exit
      char c = getchar();
   }
