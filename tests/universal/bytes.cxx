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

#include <iostream>

#include "zenoh.hxx"
#undef NDEBUG
#include <assert.h>

using namespace zenoh;

void reader_writer() {
    std::cout << "running reader_writer\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes::Writer writer;
    writer.write_all(data.data(), 5);
    writer.write_all(data.data() + 5, 5);

    Bytes b = std::move(writer).finish();
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
    std::cout << "running reader_seek_tell\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Bytes::Writer writer;
    writer.write_all(data.data(), 5);
    writer.write_all(data.data() + 5, 5);
    Bytes b = std::move(writer).finish();

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

void reader_writer_append() {
    std::cout << "running reader_writer_append\n";
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<uint8_t> data2 = {11, 12, 13, 14};
    Bytes b, b2;
    {
        Bytes::Writer writer;
        writer.write_all(data2.data(), 4);
        b2 = std::move(writer).finish();
    }
    {
        Bytes::Writer writer;
        writer.write_all(data.data(), 10);
        writer.append(std::move(b2));
        b = std::move(writer).finish();
    }

    auto reader = b.reader();
    std::vector<uint8_t> out(3);
    assert(reader.read(out.data(), 3) == 3);
    assert(out == std::vector<uint8_t>(data.begin(), data.begin() + 3));
    out = std::vector<uint8_t>(7);
    assert(reader.read(out.data(), 7) == 7);
    assert(out == std::vector<uint8_t>(data.begin() + 3, data.end()));

    out = std::vector<uint8_t>(4);
    assert(reader.read(out.data(), 4) == 4);
    assert(out == data2);
    assert(reader.read(out.data(), 1) == 0);  // reached the end of the payload
}

void from_into() {
    std::vector<uint8_t> v = {1, 2, 4, 5, 6, 7, 8, 9, 10};
    std::vector<uint8_t> v2 = v;
    std::string s = "abcdefg";
    std::string s2 = s;

    Bytes bv(v), bs(s), bv2(std::move(v2)), bs2(std::move(s2));
    assert(bv.as_vector() == v);
    assert(bs.as_string() == s);
    assert(bv2.as_vector() == v);
    assert(bs2.as_string() == s);
}

int main(int argc, char** argv) {
    reader_writer();
    reader_seek_tell();
    reader_writer_append();
    from_into();
}
