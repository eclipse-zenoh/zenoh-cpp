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

#ifdef ZENOH_CPP_EXAMPLE_WITH_PROTOBUF
#include "entity.pb.h"
#endif

#if __cplusplus >= 202002L
#include <span>
#endif

#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char** argv) {
    // Using raw data
    // String
    {
        // C-String
        {
            const char* input = "test";
            const auto payload = Bytes(input);
            const auto output = payload.as_string();
            assert(input == output);
        }
        // std::string
        {
            const std::string input = "test";
            const auto payload = Bytes(input);
            const auto output = payload.as_string();
            assert(input == output);
        }
        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
#if defined(ZENOHCXX_ZENOHC) || (Z_FEATURE_ENCODING_VALUES == 1)
        const auto encoding = Encoding::Predefined::zenoh_string();
#else
        const auto encoding = Encoding("zenoh/string");
#endif
    }

    // Vector of uint8_t
    {
        const std::vector<uint8_t> input = {1, 2, 3, 4};
        const auto payload = Bytes(input);
        const auto output = payload.as_vector();
        assert(input == output);
        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
#if defined(ZENOHCXX_ZENOHC) || (Z_FEATURE_ENCODING_VALUES == 1)
        const auto encoding = Encoding::Predefined::zenoh_bytes();
#else
        const auto encoding = Encoding("zenoh/bytes");
#endif
    }

    // Iterator RAW (in case of fragmented data)
    {
        const std::vector<uint8_t> input = {1, 2, 3, 4};
        const auto payload = Bytes(input);

        size_t idx = 0;
        auto it = payload.slice_iter();
        for (auto elem = it.next(); elem.has_value(); elem = it.next()) {
            const auto& slice = elem.value();
            for (size_t i = 0; i < slice.len; ++i) {
                assert(input[idx++] == slice.data[i]);
            }
        }
    }

    // Serialization
    // Vector
    {
        const std::vector<int32_t> input = {1, 2, 3, 4};
        const auto payload = ext::serialize(input);
        const auto output = ext::deserialize<std::vector<int32_t>>(payload);
        assert(input == output);
    }
    // HashMap
    {
        const std::unordered_map<uint64_t, std::string> input = {{0, "abc"}, {1, "def"}};
        const auto payload = ext::serialize(input);
        const auto output = ext::deserialize<std::unordered_map<uint64_t, std::string>>(payload);
        assert(input == output);
    }

    // Span
    #if __cplusplus >= 202002L
    {
      double input[] = {1.0, 2.0, 3.0, 4.0};
      const auto payload = ext::serialize(std::span(input));
      const auto output = ext::deserialize<std::vector<double>>(payload);
      assert(std::equal(std::begin(input), std::end(input), std::begin(output)));
    }
    #endif

    // Serializer, deserializer (for struct or tuple serialization)
    {
        // serialization
        auto serializer = ext::Serializer();
        const uint32_t i1 = 1234;
        const std::string i2 = "test";
        const std::vector<uint8_t> i3 = {1, 2, 3, 4};
        serializer.serialize(i1);
        serializer.serialize(i2);
        serializer.serialize(i3);
        Bytes bytes = std::move(serializer).finish();

        // deserialization
        auto deserializer = ext::Deserializer(bytes);

        const auto o1 = deserializer.deserialize<uint32_t>();
        const auto o2 = deserializer.deserialize<std::string>();
        const auto o3 = deserializer.deserialize<std::vector<uint8_t>>();

        assert(i1 == o1);
        assert(i2 == o2);
        assert(i3 == o3);
    }

#ifdef ZENOH_CPP_EXAMPLE_WITH_PROTOBUF
    // Protobuf
    // This example is conditionally compiled depending on build system being able to find Protobuf installation
    {
        // (Protobuf recommendation) Verify that the version of the library that we linked against is
        // compatible with the version of the headers we compiled against.
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        // Construct PB message
        Entity input;
        input.set_id(1234);
        input.set_name("John Doe");

        // Serialize PB message into wire format
        // Avoid using std::string for raw bytes, since certain bindigs
        // might rise a error if the string is not a valid sequence of utf-8 characters.
        std::vector<uint8_t> input_wire_pb(input.ByteSizeLong());
        input.SerializeToArray(input_wire_pb.data(), input_wire_pb.size());

        // Put PB wire format into Bytes
        const auto payload = Bytes(std::move(input_wire_pb));

        // Extract PB wire format
        const auto output_wire_pb = payload.as_vector();

        // deserialize output wire PB into PB message
        Entity output;
        const auto parsed = output.ParseFromArray(output_wire_pb.data(), output_wire_pb.size());
        assert(parsed);

        // data is equal
        assert(input.id() == output.id());
        assert(input.name() == output.name());

        // Corresponding encoding to be used in operations like `.put()`, `.reply()`, etc.
#if defined(ZENOHCXX_ZENOHC) || (Z_FEATURE_ENCODING_VALUES == 1)
        const auto encoding = Encoding::Predefined::application_protobuf();
#else
        const auto encoding = Encoding("application/protobuf");
#endif
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