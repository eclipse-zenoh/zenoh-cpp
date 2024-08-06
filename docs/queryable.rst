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
      auto queryable_keyexpr = KeyExpr("demo/example/simple");
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));
      auto queryable = session.declare_queryable(
            queryable_keyexpr, 
            [&queryable_expr](const Query& query) {
               std::cout << "Received Query '" 
                        << query.get_keyexpr().as_string_view() 
                        << "?" << query.get_parameters() << std::endl;
               query.reply(queryable_expr, Bytes::serialize("42"));
         }
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
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));

      auto on_reply = [](const Reply& reply) {
         if (reply.is_ok()) {
            auto&& sample = reply.get_ok();
            std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                      << sample.get_payload().deserialize<std::string>() << "')\n";
         } else {
            auto&& err = reply.get_err();
            std::cout << "Received an error :" 
                      << error.get_payload().deserialzie<std::string>() << "\n";
         }
      };

      auto on_done = []() {
         std::cout << "No more replies" << std::endl;
      };

      // Send a query and pass two callbacks: one for processing the reply 
      // and one for handle the end of replies
      session.get(KeyExpr("demo/example/simple"), "", on_reply, on_done);

      // Do not exit immediately, give time to process replies
      // Better to wait on a mutex signalled by the on_done callback
      c = getchar();
   }

Client example using blocking stream interface. Notice that reply callback may receive error message instead of a sample.
Also notice that the callback is processed asynchronously, so the client must not exit immediately.

.. code-block:: c++

   #include "zenoh.hxx"
   using namespace zenoh;

   int main(int argc, char **argv) {
      Config config = Config::create_default();
      auto session = Session::open(std::move(config));

      // Send a query and receive a stream providing replies.
      // We will receive a FIFO buffer to store unprocessed replies (with size of 16).
      auto replies = session.get(KeyExpr("demo/example/simple"), "", channels::FifoChannel(16));
      while (true) {
         auto res = replies.recv();
         Reply* reply = std::get_if(&res);
         if (reply == nullptr) break;
         if (reply->is_ok()) {
            const Sample& sample = reply->get_ok();
            std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                      << sample.get_payload().deserialize<std::string>() << "')\n";
         } else {
            const ReplyError& error = reply->get_err();
            std::cout << "Received an error :" 
                      << error.get_payload().deserialzie<std::string>() << "\n";
         }
         
      }

      std::cout << "No more replies" << std::endl;
   }
