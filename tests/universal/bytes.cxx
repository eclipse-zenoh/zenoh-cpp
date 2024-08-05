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
using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void reader_writer() {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b;
    {
        auto writer = b.writer();
        writer.write(data.data(), 5);
        writer.write(data.data() + 5, 5);
    }

    auto reader = b.reader();
    std::vector<uint8_t> out(3);
    assert(reader.read(out.data(), 3) == 3);
    assert(out == std::vector<uint8_t>(data.begin(), data.begin() + 3));
    out = std::vector<uint8_t>(7);
    assert(reader.read(out.data(), 10) == 7);
    assert(out == std::vector<uint8_t>(data.begin() + 3, data.end()));
    assert(reader.read(out.data(), 10) == 0);
}

void reader_seek_tell() {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b;
    {
        auto writer = b.writer();
        writer.write(data.data(), 5);
        writer.write(data.data() + 5, 5);
    }

    auto reader = b.reader();
    assert(reader.tell() == 0);
    uint8_t i = 255;
    reader.read(&i, 1);
    assert(i == 0);
    assert(reader.tell() == 1);
    reader.seek_from_current(5);
    assert(reader.tell() == 6);
    reader.read(&i, 1);
    assert(i == 6);
    reader.seek_from_start(3);
    assert(reader.tell() == 3);
    reader.read(&i, 1);
    assert(i == 3);

    reader.seek_from_end(-3);
    assert(reader.tell() == 7);
    reader.read(&i, 1);
    assert(i == 7);

    reader.seek_from_current(-2);
    assert(reader.tell() == 6);
    reader.read(&i, 1);
    assert(i == 6);
}

void serde_basic() {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes b = Bytes::serialize(data);
    assert(b.deserialize<std::vector<uint8_t>>() == data);

    b = Bytes::serialize(std::make_pair(data.data(), data.size()));
    assert(b.deserialize<std::vector<uint8_t>>() == data);

    // verify that no-copy serialization only forwards a pointer, and thus all data modifications
    // will likely have visible side effects
    b = Bytes::serialize(data, ZenohCodec<ZenohCodecType::AVOID_COPY>());
    data[0] = 100;
    data[9] = 200;
    assert(b.deserialize<std::vector<uint8_t>>() == data);

    std::string s = "abc";
    b = Bytes::serialize(s);
    assert(b.deserialize<std::string>() == s);

#define __ZENOH_TEST_ARITHMETIC(TYPE, VALUE) \
    {                                        \
        TYPE t = VALUE;                      \
        Bytes b = Bytes::serialize(t);       \
        assert(b.deserialize<TYPE>() == t);  \
    }

    __ZENOH_TEST_ARITHMETIC(uint8_t, 5);
    __ZENOH_TEST_ARITHMETIC(uint16_t, 500);
    __ZENOH_TEST_ARITHMETIC(uint32_t, 50000);
    __ZENOH_TEST_ARITHMETIC(uint64_t, 500000000000);

    __ZENOH_TEST_ARITHMETIC(int8_t, -5);
    __ZENOH_TEST_ARITHMETIC(int16_t, 500);
    __ZENOH_TEST_ARITHMETIC(int32_t, -50000);
    __ZENOH_TEST_ARITHMETIC(int64_t, -500000000000);

    __ZENOH_TEST_ARITHMETIC(float, 0.5f);
    __ZENOH_TEST_ARITHMETIC(double, 123.45);

    auto p = std::make_pair(-12, std::string("123"));
    b = Bytes::serialize(p);
    assert(b.deserialize<decltype(p)>() == p);
}

void serde_iter() {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto b = Bytes::serialize_from_iter(data.begin(), data.end());
    auto it = b.iter();
    std::vector<uint8_t> out;
    for (auto bb = it.next(); bb.has_value(); bb = it.next()) {
        out.push_back(bb->deserialize<uint8_t>());
    }
    assert(data == out);
}

void serde_advanced() {
    std::vector<float> v = {0.1f, 0.2f, 0.3f};
    auto b = Bytes::serialize(v);
    assert(b.deserialize<decltype(v)>() == v);

    std::unordered_map<std::string, double> m = {{"a", 0.5}, {"b", -123.45}, {"abc", 3.1415926}};
    b = Bytes::serialize(m);
    assert(b.deserialize<decltype(m)>() == m);

    std::set<uint8_t> s = {1, 2, 3, 4, 0};
    b = Bytes::serialize(s);
    assert(b.deserialize<decltype(s)>() == s);

    std::map<std::string, std::deque<double>> m2 = {
        {"a", {0.5, 0.2}}, {"b", {-123.45, 0.4}}, {"abc", {3.1415926, -1.0}}};

    b = Bytes::serialize(m2);
    assert(b.deserialize<decltype(m2)>() == m2);
}

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
    template <class T>
    static T deserialize(const Bytes& b, ZResult* err = nullptr);

   private:
    template <std::uint8_t T_numBytes>
    using UintType = typename std::conditional<
        T_numBytes == 1, std::uint8_t,
        typename std::conditional<T_numBytes == 2, std::uint16_t,
                                  typename std::conditional<T_numBytes == 3 || T_numBytes == 4, std::uint32_t,
                                                            std::uint64_t>::type>::type>::type;

    template <class T>
    static std::enable_if_t<std::is_arithmetic_v<T>, std::array<uint8_t, sizeof(T)>> serialize_arithmetic(T t) {
        // use simple little endian encoding
        std::array<uint8_t, sizeof(T)> out;
        uint8_t mask = 0b11111111u;
        UintType<sizeof(T)> u = reinterpret_cast<UintType<sizeof(T)>&>(t);
        for (size_t i = 0; i < out.size(); i++) {
            out[i] = static_cast<uint8_t>(u & mask);
            u = u >> 8;
        }
        return out;
    }

    template <class T>
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

template <>
CustomStruct CustomCodec::deserialize<CustomStruct>(const Bytes& b, ZResult* err) {
    CustomStruct out;
    if (b.size() < 12) {  // we should have at least 12 bytes in the payload
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

void serde_custom() {
    CustomStruct s;
    s.d = 0.5;
    s.u = 500;
    s.s = "abcd";
    auto b = Bytes::serialize(s, CustomCodec());
    CustomStruct out = b.deserialize<CustomStruct>(CustomCodec());
    assert(s.d == out.d);
    assert(s.u == out.u);
    assert(s.s == out.s);
}

int main(int argc, char** argv) {
    reader_writer();
    reader_seek_tell();
    serde_basic();
    serde_iter();
    serde_advanced();
    serde_custom();
}
