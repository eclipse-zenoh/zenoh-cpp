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
#undef NDEBUG
#include <assert.h>

using namespace zenoh;

template <class T>
bool zenoh_test_serialization(const T& t) {
    Bytes b = ext::serialize(t);
    return ext::deserialize<T>(b) == t;
}

void serialize_primitive() {
    assert(zenoh_test_serialization<uint8_t>(5));
    assert(zenoh_test_serialization<uint16_t>(500));
    assert(zenoh_test_serialization<uint32_t>(50000));
    assert(zenoh_test_serialization<uint64_t>(500000000000));

    assert(zenoh_test_serialization<int8_t>(-5));
    assert(zenoh_test_serialization<int16_t>(500));
    assert(zenoh_test_serialization<int32_t>(50000));
    assert(zenoh_test_serialization<int64_t>(500000000000));

    assert(zenoh_test_serialization<float>(0.5f));
    assert(zenoh_test_serialization<double>(123.45));

    assert(zenoh_test_serialization<bool>(true));
    assert(zenoh_test_serialization<bool>(false));
}

void serialize_tuple() {
    std::tuple<double, std::string, uint8_t> t = {0.5, "test", 1};
    assert(zenoh_test_serialization(t));

    std::pair<std::vector<int16_t>, float> p = {{-1, -10000, 10000, 1}, 3.1415926f};
    assert(zenoh_test_serialization(p));
}

void serialize_container() {
    assert(zenoh_test_serialization<std::string>("abcdefg"));
    assert(zenoh_test_serialization<std::vector<float>>({0.1f, 0.2f, -0.5f, 1000.578f}));
    assert(zenoh_test_serialization<std::unordered_set<int32_t>>({1, 2, 3, -5, 10000, -999999999}));
    assert(zenoh_test_serialization<std::set<int32_t>>({1, 2, 3, -5, 10000, -999999999}));
    std::array<int16_t, 6> a = {1, 2, 3, -5, 5, -500};
    assert(zenoh_test_serialization(a));
    std::unordered_map<uint64_t, std::string> m;
    m[100] = "abc";
    m[10000] = "def";
    m[2000000000] = "hij";
    assert(zenoh_test_serialization(m));
    std::map<uint64_t, std::string> m2;
    m2[100] = "abc";
    m2[10000] = "def";
    m2[2000000000] = "hij";
    assert(zenoh_test_serialization(m2));
}

struct CustomStruct {
    std::vector<double> vd;
    int32_t i;
    std::string s;
};

bool __zenoh_serialize_with_serializer(zenoh::ext::Serializer& serializer, const CustomStruct& s, ZResult* err) {
    return zenoh::ext::detail::serialize_with_serializer(serializer, s.vd, err) &&
           zenoh::ext::detail::serialize_with_serializer(serializer, s.i, err) &&
           zenoh::ext::detail::serialize_with_serializer(serializer, s.s, err);
}

bool __zenoh_deserialize_with_deserializer(zenoh::ext::Deserializer& deserializer, CustomStruct& s, ZResult* err) {
    return zenoh::ext::detail::deserialize_with_deserializer(deserializer, s.vd, err) &&
           zenoh::ext::detail::deserialize_with_deserializer(deserializer, s.i, err) &&
           zenoh::ext::detail::deserialize_with_deserializer(deserializer, s.s, err);
}

void serialize_custom() {
    CustomStruct s = {{0.1, 0.2, -1000.55}, 32, "test"};
    Bytes b = zenoh::ext::serialize(s);
    CustomStruct s_out = zenoh::ext::deserialize<CustomStruct>(b);
    assert(s.vd == s_out.vd);
    assert(s.i == s_out.i);
    assert(s.s == s_out.s);
}

template <class T>
bool check_serialization(const T& value, const std::vector<uint8_t>& out) {
    return ext::serialize(value).as_vector() == out;
}

void binary_format_test() {
    int32_t i1 = 1234566, i2 = -49245;
    assert(check_serialization(i1, {134, 214, 18, 0}));
    assert(check_serialization(i2, {163, 63, 255, 255}));

    std::string s = "test";
    assert(check_serialization(s, {4, 116, 101, 115, 116}));

    std::tuple<uint16_t, float, std::string> t(500, 1234.0f, "test");
    assert(check_serialization(t, {244, 1, 0, 64, 154, 68, 4, 116, 101, 115, 116}));

    std::vector<int64_t> v = {-100, 500, 100000, -20000000};
    assert(check_serialization(v, {4,   156, 255, 255, 255, 255, 255, 255, 255, 244, 1,   0,   0,   0,   0,   0,  0,
                                   160, 134, 1,   0,   0,   0,   0,   0,   0,   211, 206, 254, 255, 255, 255, 255}));

    std::vector<std::pair<std::string, int16_t>> vp = {{"s1", 10}, {"s2", -10000}};
    assert(check_serialization(vp, {2, 2, 115, 49, 10, 0, 2, 115, 50, 240, 216}));
}

int main(int argc, char** argv) {
    serialize_primitive();
    serialize_tuple();
    serialize_container();
    serialize_custom();
    binary_format_test();
}
