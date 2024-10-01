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
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));
      // Publish without creating a Publisher object
      session.put(KeyExpr("demo/example/simple"), Bytes::serialize("Simple!"));

      // Publish from a Publisher object
      auto publisher = session.declare_publisher(KeyExpr("demo/example/simple"));
      publisher.put("Simple!");
   }

Subscriber example:

.. code-block:: c++

   #include "zenoh.hxx"
   #include <iostream>
   using namespace zenoh;

   int main(int argc, char **argv) {
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));
      auto subscriber = session.declare_subscriber(
         KeyExpr("demo/example/simple"), 
         [](const Sample& sample) {
            std::cout << "Received: " << sample.get_payload().deserialize<std::string>() << std::endl;
         }
      );
      // Wait for a key press to exit
      char c = getchar();
   }

Subscriber example with non-blocking stream interface:

.. code-block:: c++

   #include "zenoh.hxx"
   #include <iostream>
   #include <thread>
   #include <chrono>
   using namespace zenoh;
   using namespace std::chrono_literals;

   int main(int argc, char **argv) {
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));
      auto subscriber = session.declare_subscriber(
         KeyExpr("demo/example/simple"),
         channels::FifoChannel(16), // use FIFO buffer to store unprocessed messages 
      );
      while (true) {
         auto res = subscriber.handler().try_recv()
         if (std::holds_alternative<Sample>(res)) {
            std::cout << "Received: " << std::get<Sample>(res).get_payload().deserialize<std::string>() << std::endl;
         } else if (std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA) {
            std::this_thread::sleep_for(1s); // do some other work
         } else {
            break; // channel is closed
         }
      }
   }
