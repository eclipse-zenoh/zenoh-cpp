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
#include <thread>
#include <chrono>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

struct CustomStruct {
    uint32_t u = 0;
    double d = 0;
    std::string s = {};
};

// Example of codec for a custom class / struct
// We need to define corresponding serialize and deserialize methods
struct CustomCodec {
    static Bytes serialize(const CustomStruct& s) {
        Bytes b;
        auto writer = b.writer();
        writer.write(serialize_arithmetic(s.u).data(), 4);
        writer.write(serialize_arithmetic(s.d).data(), 8);
        writer.write(reinterpret_cast<const uint8_t*>(s.s.data()), s.s.size());
        return b;
    }

    // deserialize should be a template method
    template<class T>
    static T deserialize(const Bytes& b, ZError* err = nullptr);

private:
    template <std::uint8_t T_numBytes>
    using UintType =
        typename std::conditional<T_numBytes == 1, std::uint8_t,
            typename std::conditional<T_numBytes == 2, std::uint16_t,
                typename std::conditional<T_numBytes == 3 || T_numBytes == 4, std::uint32_t,
                    std::uint64_t
                >::type
            >::type
        >::type;
    
    template<class T>
    static  std::enable_if_t<std::is_arithmetic_v<T>, std::array<uint8_t, sizeof(T)>> serialize_arithmetic(T t) {
        // use simple little endian encoding
        std::array<uint8_t, sizeof(T)> out;
        uint8_t mask = 0b11111111;
        UintType<sizeof(T)> u = reinterpret_cast<UintType<sizeof(T)>&>(t);
        for (size_t i = 0; i < out.size(); i++) {
            out[i] = static_cast<uint8_t>(u & mask);
            u = u >> 8;
        }
        return out;
    }

    template<class T>
    static std::enable_if_t<std::is_arithmetic_v<T>, T> deserialize_arithmetic(const uint8_t* buf) {
        // use simple little endian encoding
        UintType<sizeof(T)> out = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            out = out << 8;
            out = out | buf[sizeof(T) - i - 1];
        }
        return reinterpret_cast<const T&>(out);
    }
};

template<>
CustomStruct CustomCodec::deserialize<CustomStruct>(const Bytes& b, ZError* err) {
    CustomStruct out;
    if (b.size() < 12) { // we should have at least 12 bytes in the payload
        if (err != nullptr) {
            *err = -1; 
            return out;
        } else {
            throw std::runtime_error("Insufficient payload size");
        }
    }
    
    std::array<uint8_t, 8> buf;
    auto reader = b.reader();
    
    reader.read(buf.data(), 4);
    out.u = deserialize_arithmetic<uint32_t>(buf.data());
    reader.read(buf.data(), 8);
    out.d = deserialize_arithmetic<double>(buf.data());
    size_t remaining = b.size() - 12;
    out.s = std::string(remaining, 0);
    reader.read(reinterpret_cast<uint8_t*>(out.s.data()), remaining);
    return out;
}


class CustomPublisher {
public:
    CustomPublisher(Session& session, std::string_view key_expr) 
        : _pub(session.declare_publisher(KeyExpr(key_expr))) {;
    }

    void put(const CustomStruct& s) { 
        _pub.put(Bytes::serialize(s, CustomCodec())); 
    }

private:
    Publisher _pub;
};

class CustomSubscriber {
public:
    CustomSubscriber(Session& session, std::string_view key_expr) 
        : _sub(
            session.declare_subscriber(
                KeyExpr(key_expr), [this](const Sample& s) { this->on_receive(s); }, closures::none
            )
         ) {;
    }

private:
    Subscriber<void> _sub;
    
    void on_receive(const Sample& sample) { 
        CustomStruct s = sample.get_payload().deserialize<CustomStruct>(CustomCodec());
        std::cout << "Received: " << "{" << s.u << ", " << s.d << ", " << s.s  << "}\n"; 
    }
};

int main(int, char**) {
    try {
        Config config = Config::create_default();
        auto session = Session::open(std::move(config));

        std::string keyexpr = "demo/example/simple";
        CustomPublisher pub(session, keyexpr);
        CustomSubscriber sub(session, keyexpr);

        pub.put({0, 0.5, "abc"});
        std::this_thread::sleep_for(1s); /// wait a bit to receive the message

    } catch (std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}