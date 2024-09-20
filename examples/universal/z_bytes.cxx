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

#ifdef ZENOH_CPP_EXMAPLE_WITH_PROTOBUF
#include "test.pb.h"
#endif

#include "stdio.h"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char** argv) {
    // Numeric: u8, u16, u32, u128, usize, i8, i16, i32, i128, isize, f32, f64
    {
        const uint32_t input = 1234;
        const auto payload = Bytes::serialize(input);
        const auto output = payload.deserialize<uint32_t>();
        assert(input == output);
        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
        const auto encoding = Encoding("zenoh/uint32");
    }

    // String
    {
        // C-String
        {
            const char* input = "test";
            const auto payload = Bytes::serialize(input);
            const auto output = payload.deserialize<std::string>();
            assert(input == output);
        }
        // std::string
        {
            const std::string input = "test";
            const auto payload = Bytes::serialize(input);
            const auto output = payload.deserialize<std::string>();
            assert(input == output);
        }
        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
        const auto encoding = Encoding("zenoh/string");
    }

    // Vec<u8>: The deserialization should be infallible
    {
        const std::vector<uint8_t> input = {1, 2, 3, 4};
        const auto payload = Bytes::serialize(input);
        const auto output = payload.deserialize<std::vector<uint8_t>>();
        assert(input == output);
        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
        const auto encoding = Encoding("zenoh/bytes");
    }

    // Writer & Reader
    {
        // serialization
        Bytes bytes;
        auto writer = bytes.writer();

        const uint32_t i1 = 1234;
        const std::string i2 = "test";
        const std::vector<uint8_t> i3 = {1, 2, 3, 4};

        writer.append_bounded(i1);
        writer.append_bounded(i2);
        writer.append_bounded(i3);

        // deserialization
        auto reader = bytes.reader();

        const auto o1 = reader.read_bounded().deserialize<uint32_t>();
        const auto o2 = reader.read_bounded().deserialize<std::string>();
        const auto o3 = reader.read_bounded().deserialize<std::vector<uint8_t>>();

        assert(i1 == o1);
        assert(i2 == o2);
        assert(i3 == o3);
    }

    // Iterator
    {
        const int32_t input[] = {1, 2, 3, 4};
        const auto payload = Bytes::serialize_from_iter(input, input + 4);

        auto idx = 0;
        auto it = payload.iter();
        for (auto elem = it.next(); elem.has_value(); elem = it.next()) {
            assert(input[idx++] == elem.value().deserialize<int32_t>());
        }
    }

    // Iterator RAW
    {
        const std::vector<uint8_t> input = {1, 2, 3, 4};
        const auto payload = Bytes::serialize(input);

        size_t idx = 0;
        auto it = payload.slice_iter();
        for (auto elem = it.next(); elem.has_value(); elem = it.next()) {
            const auto& slice = elem.value();
            for (size_t i = 0; i < slice.len; ++i) {
                assert(input[idx++] == slice.data[i]);
            }
        }
    }

    // HashMap
    {
        const std::unordered_map<uint64_t, std::string> input = {{0, "abc"}, {1, "def"}};
        const auto payload = Bytes::serialize(input);
        const auto output = payload.deserialize<std::unordered_map<uint64_t, std::string>>();
        assert(input == output);
    }

#ifdef ZENOH_CPP_EXMAPLE_WITH_PROTOBUF
    // Protobuf
    // This example is conditionally compiled depending on build system being able to find Protobuf installation
    {
        // (Protobuf recommendation) Verify that the version of the library that we linked against is
        // compatible with the version of the headers we compiled against.
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        // Construct PB message
        Book input;
        input.set_author("H. P. Lovecraft");
        input.set_title("The Call of Cthulhu");
        input.set_isbn(931082);

        // Serialize PB message into wire format
        const auto input_wire_pb = input.SerializeAsString();

        // Put PB wire format into Bytes
        const auto payload = Bytes::serialize(input_wire_pb);

        // Extract PB wire format
        const auto output_wire_pb = payload.deserialize<std::string>();

        // wire PB data is equal
        assert(input_wire_pb == output_wire_pb);

        // deserialize output wire PB into PB message
        Book output;
        const auto parsed = output.ParseFromString(output_wire_pb);
        assert(parsed);

        // data is equal
        assert(input.author() == output.author());
        assert(input.title() == output.title());
        assert(input.isbn() == output.isbn());

        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
        const auto encoding = Encoding("application/protobuf");

        // (Protobuf recommendation) Optional:  Delete all global objects allocated by libprotobuf.
        google::protobuf::ShutdownProtobufLibrary();
    }
#endif

    return 0;
}

int main(int argc, char** argv) {
    try {
#ifdef ZENOHCXX_ZENOHC
        init_log_from_env_or("error");
#endif
        return _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}