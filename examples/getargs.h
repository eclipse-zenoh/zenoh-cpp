//
// Copyright (c) 2023 ZettaScale Technology
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
#pragma once

#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "zenoh.hxx"

struct CmdArg {
    const char *name;
    const char **value;
};

inline void getargs(int argc, char **argv, const std::vector<CmdArg> &required,
                    const std::vector<CmdArg> &optional = {},
                    const std::unordered_map<std::string, CmdArg> &named = {}) {
    // Show help if help option is passed or if no parameters are passed when some are required
    if ((argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) ||
        argc == 1 && required.size() > 0) {
        bool defaults = false;
        std::cout << "Usage" << std::endl;
        std::cout << "  " << argv[0];
        for (auto &p : required) {
            std::cout << " " << p.name;
        }
        for (auto &p : optional) {
            std::cout << " [" << p.name << "]";
            if (*p.value) defaults = true;
        }
        for (auto &p : named) {
            std::cout << " " << p.first << " [" << p.second.name << "]";
            if (*p.second.value) defaults = true;
        }
        std::cout << std::endl;

        if (defaults) {
            std::cout << "Defaults" << std::endl;
            for (auto &p : optional) {
                if (*p.value) {
                    std::cout << "  " << p.name << " = " << *p.value << std::endl;
                }
            }
            for (auto &p : named) {
                if (*p.second.value) {
                    std::cout << "  " << p.second.name << " = " << *p.second.value << std::endl;
                }
            }
        }
        exit(0);
    }

    int position = 0;
    const char **destination = nullptr;
    std::vector<CmdArg> positioned = required;
    positioned.insert(positioned.end(), optional.begin(), optional.end());
    for (int i = 1; i < argc; ++i) {
        if (destination) {
            *destination = argv[i];
            destination = nullptr;
            continue;
        }
        auto param = named.find(argv[i]);
        if (param != named.end()) {
            destination = param->second.value;
        } else if (position < positioned.size()) {
            *positioned[position].value = argv[i];
            ++position;
        } else {
            std::cout << "Unexpected parameter: " << argv[i] << std::endl;
            exit(-1);
        }
    }
    if (position < required.size()) {
        std::cout << "Missing required parameter: " << required[position].name << std::endl;
        exit(-1);
    }
}

inline zenoh::Config parse_args(int argc, char **argv, const std::vector<CmdArg> &required,
                                const std::vector<CmdArg> &optional = {},
                                const std::unordered_map<std::string, CmdArg> &named = {}) {
    std::unordered_map<std::string, CmdArg> named_with_config = named;
#ifdef ZENOHCXX_ZENOHC
    const char *config_file = nullptr;
    named_with_config.emplace("-c", CmdArg{"config file", &config_file});
#endif
    const char *locator = nullptr;
    named_with_config.emplace("-l", CmdArg{"locator to listen on", &locator});
    const char *endpoint = nullptr;
    named_with_config.emplace("-e", CmdArg{"endpoint to connect to", &endpoint});
#ifdef ZENOHCXX_ZENOHC
    const char *mode = "peer";
#elif defined(ZENOHCXX_ZENOHPICO)
    const char *mode = "client";
#endif
    named_with_config.emplace("-m", CmdArg{"mode (peer | client)", &mode});

    getargs(argc, argv, required, optional, named_with_config);
    zenoh::Config config = zenoh::Config::create_default();

    if (mode != nullptr && strcmp(mode, "peer") != 0 && strcmp(mode, "client") != 0) {
        throw std::runtime_error("Mode can only be 'peer' or 'client'");
    }

#ifdef ZENOHCXX_ZENOHC
    if (config_file) {
        config = zenoh::Config::from_file(config_file);
    }
    if (locator) {
        config.insert_json5(Z_CONFIG_CONNECT_KEY, std::string("[\"") + locator + "\"]");
    }
    if (mode) {
        config.insert_json5(Z_CONFIG_MODE_KEY, std::string("'") + mode + "'");
    }
#elif defined(ZENOHCXX_ZENOHPICO)
    if (mode) {
        config.insert(Z_CONFIG_MODE_KEY, mode);
        if (strcmp(mode, "peer") == 0) {
            if (locator == nullptr) {
                throw std::runtime_error(
                    "Zenoh-Pico in 'peer' mode requires providing a multicast group locator to listen to (-l option), "
                    "e. g. 'udp/224.0.0.224:7447#iface=lo'");
            } else {
                config.insert(Z_CONFIG_LISTEN_KEY, locator);
            }
        } else if (strcmp(mode, "client") == 0) {
            if (endpoint != nullptr) {
                config.insert(Z_CONFIG_CONNECT_KEY, endpoint);
            }
        }
    }
#endif
    return std::move(config);
}