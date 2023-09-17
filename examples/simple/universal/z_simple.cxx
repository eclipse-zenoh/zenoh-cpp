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
    CustomerClass(CustomerClass&&) = delete;
    CustomerClass(const CustomerClass&) = delete;
    CustomerClass& operator=(const CustomerClass&) = delete;
    CustomerClass& operator=(CustomerClass&&) = delete;
    // This example also demomstrates the usage of nullptr constructor if it's necessary to postpone
    // initialization of the member variable
    CustomerClass(Session& session, const KeyExprView& keyexpr) : pub(nullptr) {
        pub = expect<Publisher>(session.declare_publisher(keyexpr));
    }

    void put(const BytesView& value) { pub.put(value); }

   private:
    Publisher pub;
};

int main(int, char**) {
    try {
        Config config;
        auto session = expect<Session>(open(std::move(config)));
        std::string keyexpr = "demo/example/simple";
        std::string value = "Simple!";
        CustomerClass customer(session, keyexpr);
        customer.put(value);
    } catch (ErrorMessage e) {
        std::cout << "Error: " << e.as_string_view() << std::endl;
    }
}