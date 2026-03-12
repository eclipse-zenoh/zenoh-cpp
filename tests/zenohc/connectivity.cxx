//
// Copyright (c) 2026 ZettaScale Technology
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

#include <chrono>
#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

Config create_config(const char* mode, const char* listen, const char* connect) {
    auto config = Config::create_default();
    config.insert_json5("mode", mode);
    config.insert_json5("scouting/multicast/enabled", "false");
    config.insert_json5("scouting/gossip/enabled", "false");
    config.insert_json5("listen/endpoints", listen);
    config.insert_json5("connect/endpoints", connect);
    return config;
}

std::pair<Session, Session> create_session_pair(const char* port) {
    std::string listen = std::string("[\"tcp/127.0.0.1:") + port + "\"]";
    auto s1 = Session::open(create_config("\"router\"", listen.c_str(), "[]"));
    std::this_thread::sleep_for(1s);
    std::string connect = std::string("[\"tcp/127.0.0.1:") + port + "\"]";
    auto s2 = Session::open(create_config("\"peer\"", "[]", connect.c_str()));
    std::this_thread::sleep_for(1s);
    return {std::move(s1), std::move(s2)};
}

void test_info_zid() {
    printf("=== test_info_zid ===\n");
    auto [router, peer] = create_session_pair("17447");

    auto router_zid = router.get_zid();
    auto peer_zid = peer.get_zid();
    assert(!(router_zid == peer_zid));

    auto routers_of_router = router.get_routers_z_id();
    assert(routers_of_router.empty());

    auto peers_of_router = router.get_peers_z_id();
    assert(peers_of_router.size() == 1);
    assert(peers_of_router[0] == peer_zid);

    auto routers_of_peer = peer.get_routers_z_id();
    assert(routers_of_peer.size() == 1);
    assert(routers_of_peer[0] == router_zid);

    auto peers_of_peer = peer.get_peers_z_id();
    assert(peers_of_peer.empty());

    printf("PASS\n\n");
}

#if defined(Z_FEATURE_UNSTABLE_API)

void test_transports_and_links() {
    printf("=== test_transports_and_links ===\n");
    auto [s1, s2] = create_session_pair("17448");

    auto transports = s1.get_transports();
    assert(transports.size() == 1);
    assert(transports[0].get_zid() == s2.get_zid());

    auto links = s1.get_links();
    assert(links.size() == 1);
    assert(links[0].get_zid() == s2.get_zid());

    printf("PASS\n\n");
}

void test_links_filtered() {
    printf("=== test_links_filtered ===\n");
    auto [s1, s2] = create_session_pair("17449");

    auto t1 = s1.get_transports();
    auto t2 = s2.get_transports();
    assert(t1.size() == 1);
    assert(t2.size() == 1);

    // Filter by s1's transport - should find links
    auto links = s1.get_links(std::move(t1[0]));
    assert(links.size() == 1);

    // Filter by s2's transport - should find no links on s1
    links = s1.get_links(std::move(t2[0]));
    assert(links.empty());

    printf("PASS\n\n");
}

void test_transport_events() {
    printf("=== test_transport_events ===\n");
    auto s1 = Session::open(create_config("\"peer\"", "[\"tcp/127.0.0.1:17450\"]", "[]"));

    std::vector<std::pair<SampleKind, Id>> events;
    auto listener = s1.declare_transport_events_listener(
        [&events](TransportEvent& e) {
            events.emplace_back(e.get_kind(), e.get_transport().get_zid());
        },
        closures::none, {.history = false});

    assert(events.empty());

    auto s2 = Session::open(create_config("\"peer\"", "[]", "[\"tcp/127.0.0.1:17450\"]"));
    std::this_thread::sleep_for(2s);

    assert(events.size() == 1);
    assert(events[0].first == SampleKind::Z_SAMPLE_KIND_PUT);
    assert(events[0].second == s2.get_zid());

    s2.close();
    std::this_thread::sleep_for(2s);

    assert(events.size() == 2);
    assert(events[1].first == SampleKind::Z_SAMPLE_KIND_DELETE);

    std::move(listener).undeclare();
    printf("PASS\n\n");
}

void test_transport_events_history() {
    printf("=== test_transport_events_history ===\n");
    auto [s1, s2] = create_session_pair("17451");

    std::vector<SampleKind> events;
    auto listener = s1.declare_transport_events_listener(
        [&events](TransportEvent& e) { events.push_back(e.get_kind()); }, closures::none, {.history = true});

    std::this_thread::sleep_for(1s);

    assert(events.size() == 1);
    assert(events[0] == SampleKind::Z_SAMPLE_KIND_PUT);

    std::move(listener).undeclare();
    printf("PASS\n\n");
}

void test_transport_events_background() {
    printf("=== test_transport_events_background ===\n");
    auto s1 = Session::open(create_config("\"peer\"", "[\"tcp/127.0.0.1:17452\"]", "[]"));

    std::vector<SampleKind> events;
    s1.declare_background_transport_events_listener(
        [&events](TransportEvent& e) { events.push_back(e.get_kind()); }, closures::none);

    auto s2 = Session::open(create_config("\"peer\"", "[]", "[\"tcp/127.0.0.1:17452\"]"));
    std::this_thread::sleep_for(2s);

    assert(events.size() == 1);
    assert(events[0] == SampleKind::Z_SAMPLE_KIND_PUT);

    printf("PASS\n\n");
}

void test_link_events() {
    printf("=== test_link_events ===\n");
    auto s1 = Session::open(create_config("\"peer\"", "[\"tcp/127.0.0.1:17453\"]", "[]"));

    std::vector<std::pair<SampleKind, Id>> events;
    auto listener = s1.declare_link_events_listener(
        [&events](LinkEvent& e) { events.emplace_back(e.get_kind(), e.get_link().get_zid()); }, closures::none,
        {.history = false});

    assert(events.empty());

    auto s2 = Session::open(create_config("\"peer\"", "[]", "[\"tcp/127.0.0.1:17453\"]"));
    std::this_thread::sleep_for(2s);

    assert(events.size() == 1);
    assert(events[0].first == SampleKind::Z_SAMPLE_KIND_PUT);
    assert(events[0].second == s2.get_zid());

    s2.close();
    std::this_thread::sleep_for(2s);

    assert(events.size() == 2);
    assert(events[1].first == SampleKind::Z_SAMPLE_KIND_DELETE);

    std::move(listener).undeclare();
    printf("PASS\n\n");
}

void test_link_events_history() {
    printf("=== test_link_events_history ===\n");
    auto [s1, s2] = create_session_pair("17454");

    std::vector<SampleKind> events;
    auto listener = s1.declare_link_events_listener(
        [&events](LinkEvent& e) { events.push_back(e.get_kind()); }, closures::none, {.history = true});

    std::this_thread::sleep_for(1s);

    assert(events.size() == 1);
    assert(events[0] == SampleKind::Z_SAMPLE_KIND_PUT);

    std::move(listener).undeclare();
    printf("PASS\n\n");
}

void test_link_events_background() {
    printf("=== test_link_events_background ===\n");
    auto s1 = Session::open(create_config("\"peer\"", "[\"tcp/127.0.0.1:17455\"]", "[]"));

    std::vector<SampleKind> events;
    s1.declare_background_link_events_listener(
        [&events](LinkEvent& e) { events.push_back(e.get_kind()); }, closures::none);

    auto s2 = Session::open(create_config("\"peer\"", "[]", "[\"tcp/127.0.0.1:17455\"]"));
    std::this_thread::sleep_for(2s);

    assert(events.size() == 1);
    assert(events[0] == SampleKind::Z_SAMPLE_KIND_PUT);

    printf("PASS\n\n");
}

void test_link_events_filtered() {
    printf("=== test_link_events_filtered ===\n");
    auto [s1, s2] = create_session_pair("17456");

    auto t1 = s1.get_transports();
    auto t2 = s2.get_transports();
    assert(t1.size() == 1);
    assert(t2.size() == 1);

    // Filter by s1's transport with history - should get events
    std::vector<SampleKind> events1;
    auto listener1 = s1.declare_link_events_listener(
        [&events1](LinkEvent& e) { events1.push_back(e.get_kind()); }, closures::none,
        {.history = true, .transport = std::move(t1[0])});

    std::this_thread::sleep_for(1s);
    assert(events1.size() >= 1);
    assert(events1[0] == SampleKind::Z_SAMPLE_KIND_PUT);
    std::move(listener1).undeclare();

    // Filter by s2's transport - should get no events on s1
    std::vector<SampleKind> events2;
    auto listener2 = s1.declare_link_events_listener(
        [&events2](LinkEvent& e) { events2.push_back(e.get_kind()); }, closures::none,
        {.history = true, .transport = std::move(t2[0])});

    std::this_thread::sleep_for(1s);
    assert(events2.empty());
    std::move(listener2).undeclare();

    printf("PASS\n\n");
}

#endif

int main(int argc, char** argv) {
    init_log_from_env_or("error");

    test_info_zid();

#if defined(Z_FEATURE_UNSTABLE_API)
    test_transports_and_links();
    test_links_filtered();
    test_transport_events();
    test_transport_events_history();
    test_transport_events_background();
    test_link_events();
    test_link_events_history();
    test_link_events_background();
    test_link_events_filtered();
    printf("All connectivity tests passed!\n");
#else
    printf("Skipping unstable API tests\n");
#endif

    return 0;
}
