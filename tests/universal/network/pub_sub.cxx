//
// Copyright (c) 2024 ZettaScale Technology
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

#include "zenoh.hxx"
#include <thread>

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

void pub_sub() {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());
    auto publisher = session1.declare_publisher(ke);

    std::this_thread::sleep_for(1s);

    bool subscriber_dropped = false;
    std::vector<std::pair<std::string, std::string>> received_messages;
    {
        auto subscriber = session2.declare_subscriber(
            ke, 
            [&received_messages](const Sample& s) { 
                received_messages.emplace_back(s.get_keyexpr().as_string_view(), s.get_payload().deserialize<std::string>()); 
            },
            [&subscriber_dropped]() { subscriber_dropped = true; }
        );

        std::this_thread::sleep_for(1s);

        publisher.put(Bytes::serialize("first"));
        publisher.put(Bytes::serialize("second"));

        std::this_thread::sleep_for(1s);
    }

    assert(received_messages.size() == 2);
    assert(received_messages[0].first == "zenoh/test");
    assert(received_messages[0].second == "first");
    assert(received_messages[1].first == "zenoh/test");
    assert(received_messages[1].second == "second");
    assert(subscriber_dropped);
}

void put_sub() {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    std::vector<std::pair<std::string, std::string>> received_messages;

    auto subscriber = session2.declare_subscriber(
        ke, 
        [&received_messages](const Sample& s) { 
            received_messages.emplace_back(s.get_keyexpr().as_string_view(), s.get_payload().deserialize<std::string>()); 
        },
        closures::none 
    );

    std::this_thread::sleep_for(1s);

    session1.put(ke, Bytes::serialize("first"));
    session1.put(ke, Bytes::serialize("second"));

    std::this_thread::sleep_for(1s);

    assert(received_messages.size() == 2);
    assert(received_messages[0].first == "zenoh/test");
    assert(received_messages[0].second == "first");
    assert(received_messages[1].first == "zenoh/test");
    assert(received_messages[1].second == "second");
}

void put_sub_fifo_channel() {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    auto subscriber = session2.declare_subscriber(ke, channels::FifoChannel(16));

    std::this_thread::sleep_for(1s);

    session1.put(ke, Bytes::serialize("first"));
    session1.put(ke, Bytes::serialize("second"));

    std::this_thread::sleep_for(1s);

    auto msg = subscriber.handler().recv().first;
    assert(static_cast<bool>(msg));
    assert(msg.get_keyexpr() == "zenoh/test");
    assert(msg.get_payload().deserialize<std::string>() == "first");
    msg = subscriber.handler().try_recv().first;
    assert(static_cast<bool>(msg));
    assert(msg.get_keyexpr() == "zenoh/test");
    assert(msg.get_payload().deserialize<std::string>() == "second");

    msg = subscriber.handler().try_recv().first;
    assert(!static_cast<bool>(msg));
}

void put_sub_ring_channel() {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    auto subscriber = session2.declare_subscriber(ke, channels::RingChannel(1));

    std::this_thread::sleep_for(1s);

    session1.put(ke, Bytes::serialize("first"));
    session1.put(ke, Bytes::serialize("second"));

    std::this_thread::sleep_for(1s);

    auto msg = subscriber.handler().recv().first;
    assert(static_cast<bool>(msg));
    assert(msg.get_keyexpr() == "zenoh/test");
    assert(msg.get_payload().deserialize<std::string>() == "second");

    msg = subscriber.handler().try_recv().first;
    assert(!static_cast<bool>(msg));
}



int main(int argc, char** argv) {
    pub_sub();
    put_sub();
    put_sub_fifo_channel();
    put_sub_ring_channel();
}
