//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
#include <iostream>

//
// zenoh.hxx automatically selects zenoh-c or zenoh-pico C++ wrapper
// depending on ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC setting
// and places it to the zenoh namespace
//
#include <iostream>

#include "zenoh.hxx"
using namespace zenoh;

class CustomerClass {
   public:
    CustomerClass() : session(nullptr), pub(nullptr) {
        Config config;
        std::cout << "a\n";
        Session s = std::get<Session>(open(std::move(config)));
        std::cout << "b\n";
        Publisher p = std::get<Publisher>(s.declare_publisher("demo/example/simple"));
        std::cout << "c\n";
        session = std::move(s);
        std::cout << "d\n";
        pub = std::move(p);
        std::cout << "e\n";
    }

    void put(std::string_view value) { pub.put(value); }

   private:
    Session session;
    Publisher pub;
};

int main(int argc, char** argv) {
    CustomerClass customer;
    customer.put("Simple!");
}
