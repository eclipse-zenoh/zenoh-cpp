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
#include <stdio.h>

#include <iostream>
#include <thread>

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char** argv) {
    const char* value = "Get from C++";
    auto&& [config, args] = ConfigCliArgParser(argc, argv).run();

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "own id: " << session.get_zid() << std::endl;

    std::cout << "routers ids:\n";
    for (const auto zid : session.get_routers_z_id()) {
        std::cout << zid << "\n";
    }

    std::cout << "peers ids:\n";
    for (const auto zid : session.get_peers_z_id()) {
        std::cout << zid << "\n";
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    std::cout << "\ntransports:\n";
    for (const auto& transport : session.get_transports()) {
        std::cout << "  Transport\n";
        std::cout << "    zid: " << transport.get_zid() << "\n";
        std::cout << "    whatami: " << whatami_as_str(transport.get_whatami()) << "\n";
        std::cout << "    is_qos: " << (transport.is_qos() ? "true" : "false") << "\n";
        std::cout << "    is_multicast: " << (transport.is_multicast() ? "true" : "false") << "\n";
    }

    std::cout << "\nlinks:\n";
    for (const auto& link : session.get_links()) {
        std::cout << "  Link\n";
        std::cout << "    zid: " << link.get_zid() << "\n";
        std::cout << "    src: " << link.get_src() << "\n";
        std::cout << "    dst: " << link.get_dst() << "\n";
        auto group = link.get_group();
        if (group.has_value()) {
            std::cout << "    group: " << group.value() << "\n";
        }
        auto auth_id = link.get_auth_identifier();
        if (auth_id.has_value()) {
            std::cout << "    auth_identifier: " << auth_id.value() << "\n";
        }
        std::cout << "    mtu: " << link.get_mtu() << "\n";
        std::cout << "    is_streamed: " << (link.is_streamed() ? "true" : "false") << "\n";
        auto interfaces = link.get_interfaces();
        if (!interfaces.empty()) {
            std::cout << "    interfaces: [";
            for (size_t i = 0; i < interfaces.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << interfaces[i];
            }
            std::cout << "]\n";
        }
        auto priorities = link.get_priorities();
        if (priorities.has_value()) {
            std::cout << "    priorities: [" << static_cast<int>(priorities.value().first) << ", "
                      << static_cast<int>(priorities.value().second) << "]\n";
        }
        auto reliability = link.get_reliability();
        if (reliability.has_value()) {
            std::cout << "    reliability: " << static_cast<int>(reliability.value()) << "\n";
        }
    }

    std::cout << "\nConnectivity events (Press CTRL-C to quit):\n";

    session.declare_background_transport_events_listener(
        [](TransportEvent& event) {
            if (event.get_kind() == Z_SAMPLE_KIND_PUT) {
                std::cout << "[Transport Event] Opened:\n";
            } else {
                std::cout << "[Transport Event] Closed:\n";
            }
            const auto& transport = event.get_transport();
            std::cout << "    zid: " << transport.get_zid() << "\n";
            std::cout << "    whatami: " << whatami_as_str(transport.get_whatami()) << "\n";
        },
        []() {});

    auto link_listener = session.declare_link_events_listener(
        [](LinkEvent& event) {
            if (event.get_kind() == Z_SAMPLE_KIND_PUT) {
                std::cout << "[Link Event] Added:\n";
            } else {
                std::cout << "[Link Event] Removed:\n";
            }
            const auto& link = event.get_link();
            std::cout << "    zid: " << link.get_zid() << "\n";
            std::cout << "    src: " << link.get_src() << "\n";
            std::cout << "    dst: " << link.get_dst() << "\n";
        },
        []() {});

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
