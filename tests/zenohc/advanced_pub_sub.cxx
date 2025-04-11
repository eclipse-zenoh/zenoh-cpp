//
// Copyright (c) 2025 ZettaScale Technology
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

#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

std::array<std::string, 6> VALUES_TO_PUBLISH = {"test_value_1", "test_value_2", "test_value_3",
                                                "test_value_4", "test_value_5", "test_value_6"};

void test_pub_sub() {
    KeyExpr ke("zenoh/advanced_pub_sub_test");
    auto config1 = Config::create_default();
    config1.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    auto session1 = Session::open(std::move(config1));
    auto session2 = Session::open(Config::create_default());

    ext::SessionExt::AdvancedPublisherOptions opts;
    opts.cache.emplace().max_samples = VALUES_TO_PUBLISH.size();
    opts.publisher_detection = true;
    opts.sample_miss_detection.emplace();

    std::vector<std::string> received_messages;

    auto publisher = session1.ext().declare_advanced_publisher(ke, std::move(opts));

    std::this_thread::sleep_for(1s);

    for (size_t i = 0; i < VALUES_TO_PUBLISH.size() / 2; i++) {
        publisher.put(VALUES_TO_PUBLISH[i]);
    }

    std::this_thread::sleep_for(1s);

    ext::SessionExt::AdvancedSubscriberOptions sub_opts;
    sub_opts.history.emplace().detect_late_publishers = true;
    sub_opts.recovery.emplace().last_sample_miss_detection =
        ext::SessionExt::AdvancedSubscriberOptions::RecoveryOptions::PeriodicQueriesOptions{1000};
    sub_opts.subscriber_detection = true;

    auto subscriber = session2.ext().declare_advanced_subscriber(
        ke, [&received_messages](const Sample& s) { received_messages.emplace_back(s.get_payload().as_string()); },
        closures::none, std::move(sub_opts));

    std::this_thread::sleep_for(3s);

    for (size_t i = VALUES_TO_PUBLISH.size() / 2; i < VALUES_TO_PUBLISH.size(); i++) {
        publisher.put(VALUES_TO_PUBLISH[i]);
    }

    std::this_thread::sleep_for(3s);

    assert(received_messages.size() == VALUES_TO_PUBLISH.size());
    for (size_t i = 0; i < received_messages.size(); i++) {
        assert(received_messages[i] == VALUES_TO_PUBLISH[i]);
    }
}

void test_pub_sub_channels() {
    KeyExpr ke("zenoh/advanced_pub_sub_chennels_test");
    auto config1 = Config::create_default();
    config1.insert_json5(Z_CONFIG_ADD_TIMESTAMP_KEY, "true");

    auto session1 = Session::open(std::move(config1));
    auto session2 = Session::open(Config::create_default());

    ext::SessionExt::AdvancedPublisherOptions opts;
    opts.cache.emplace().max_samples = VALUES_TO_PUBLISH.size();
    opts.publisher_detection = true;
    opts.sample_miss_detection.emplace();

    auto publisher = session1.ext().declare_advanced_publisher(ke, std::move(opts));

    std::this_thread::sleep_for(1s);

    for (size_t i = 0; i < VALUES_TO_PUBLISH.size() / 2; i++) {
        publisher.put(VALUES_TO_PUBLISH[i]);
    }

    std::this_thread::sleep_for(1s);

    ext::SessionExt::AdvancedSubscriberOptions sub_opts;
    sub_opts.history.emplace().detect_late_publishers = true;
    sub_opts.recovery.emplace().last_sample_miss_detection =
        ext::SessionExt::AdvancedSubscriberOptions::RecoveryOptions::PeriodicQueriesOptions{1000};
    sub_opts.subscriber_detection = true;

    auto subscriber = session2.ext().declare_advanced_subscriber(ke, channels::FifoChannel(32), std::move(sub_opts));

    std::this_thread::sleep_for(3s);

    for (size_t i = VALUES_TO_PUBLISH.size() / 2; i < VALUES_TO_PUBLISH.size(); i++) {
        publisher.put(VALUES_TO_PUBLISH[i]);
    }

    std::this_thread::sleep_for(3s);

    for (size_t i = 0; i < VALUES_TO_PUBLISH.size(); i++) {
        auto res = subscriber.handler().try_recv();
        auto sample = std::get_if<Sample>(&res);
        assert(sample != nullptr);
        assert(sample->get_payload().as_string() == VALUES_TO_PUBLISH[i]);
    }
}

int main(int argc, char** argv) {
    test_pub_sub();
    test_pub_sub_channels();
};
