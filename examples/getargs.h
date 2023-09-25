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
#include <map>
#include <string>
#include <vector>

inline void getargs(int argc, char **argv, const std::vector<std::pair<const char *, const char **>> &required,
                    const std::vector<std::pair<const char *, const char **>> &optional = {},
                    const std::map<std::string, std::pair<const char *, const char **>> &named = {}) {
    // Show help if help option is passed or if no parameters are passed when some are required
    if ((argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) ||
        argc == 1 && required.size() > 0) {
        bool defaults = false;
        std::cout << "Usage" << std::endl;
        std::cout << "  " << argv[0];
        for (auto &p : required) {
            std::cout << " " << p.first;
            if (*p.second) defaults = true;
        }
        for (auto &p : optional) {
            std::cout << " [" << p.first << "]";
            if (*p.second) defaults = true;
        }
        for (auto &p : named) {
            std::cout << " " << p.first << " [" << p.second.first << "]";
            if (*p.second.second) defaults = true;
        }
        std::cout << std::endl;

        if (defaults) {
            std::cout << "Defaults" << std::endl;
            for (auto &p : required) {
                if (*p.second) {
                    std::cout << "  " << p.first << " = " << *p.second << std::endl;
                }
            }
            for (auto &p : optional) {
                if (*p.second) {
                    std::cout << "  " << p.first << " = " << *p.second << std::endl;
                }
            }
            for (auto &p : named) {
                if (*p.second.second) {
                    std::cout << "  " << p.second.first << " = " << *p.second.second << std::endl;
                }
            }
        }
        exit(0);
    }

    int position = 0;
    const char **destination = nullptr;
    std::vector<std::pair<const char *, const char **>> positioned = required;
    positioned.insert(positioned.end(), optional.begin(), optional.end());
    for (int i = 1; i < argc; ++i) {
        if (destination) {
            *destination = argv[i];
            destination = nullptr;
            continue;
        }
        auto param = named.find(argv[i]);
        if (param != named.end()) {
            destination = param->second.second;
        } else if (position < positioned.size()) {
            *positioned[position].second = argv[i];
            ++position;
        } else {
            std::cout << "Unexpected parameter: " << argv[i] << std::endl;
            exit(-1);
        }
    }
    if (position < required.size()) {
        std::cout << "Missing required parameter: " << required[position].first << std::endl;
        exit(-1);
    }
}
