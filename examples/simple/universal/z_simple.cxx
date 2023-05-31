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

    CustomerClass(const KeyExprView& keyexpr) : session(nullptr), pub(nullptr) {
        Config config;
        Session s = std::get<Session>(open(std::move(config)));
        session = std::move(s);
        // Publisher holds a reference to the Session, so after creating the publisher the session should
        // not be moved anymore (as well as the whole CustomerClass)
        Publisher p = std::get<Publisher>(session.declare_publisher(keyexpr));
        pub = std::move(p);
    }

    void put(const BytesView& value) { pub.put(value); }

   private:
    Session session;
    Publisher pub;
};

int main(int argc, char** argv) {
    std::string keyexpr = "demo/example/simple";
    std::string value = "Simple!";
    CustomerClass customer(keyexpr);
    customer.put(value);
}