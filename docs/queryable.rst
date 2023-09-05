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

Queryable
=========

The data query pattern is implemented by the 
:cpp:class:`zenoh::Queryable` and :cpp:func:`zenoh::Session::get`.

Queryable example:

.. code-block:: c++

   #include "zenoh.hxx"
   #include <iostream>
   using namespace zenoh;

   int main(int argc, char **argv) {
      auto queryable_keyexpr = "demo/example/simple";
      Config config;
      auto session = expect<Session>(open(std::move(config)));
      auto queryable = expect<Queryable>(
         session.declare_queryable(queryable_keyexpr, [](const Query& query) {
            std::cout << "Received Query '" 
                      << query.get_keyexpr().as_string_view() 
                      << "?" << query.get_parameters().as_string_view() << std::endl;
            query.reply(queryable_expr, "42");
         })
      );
      // Wait for a key press to exit
      c = getchar();
   }

Client example. Notice that reply callback may receive error message instead of a sample.
Also notice that the callback is processed asynchronously, so the client must not exit immediately.

.. code-block:: c++

   #include "zenoh.hxx"
   using namespace zenoh;

   int main(int argc, char **argv) {
      Config config;
      auto session = expect<Session>(open(std::move(config)));

      auto on_reply = [](Reply&& reply) {
         auto result = reply.get();
         if (auto sample = std::get_if<Sample>(&result)) {
            std::cout << "Received ('" << sample->get_keyexpr().as_string_view() << "' : '"
                      << sample->get_payload().as_string_view() << "')\n";
         } else if (auto error = std::get_if<ErrorMessage>(&result)) {
            std::cout << "Received an error :" << error->as_string_view() << "\n";
         }
      };

      auto on_done = []() {
         std::cout << "No more replies" << std::endl;
      };

      // Send a query and pass two callbacks: one for processing the reply 
      // and one for handle the end of replies
      session.get("demo/example/simple", {on_reply, on_done});

      // Do not exit immediately, give time to process replies
      // Better to wait on a mutex signalled by the on_done callback
      c = getchar();
   }
