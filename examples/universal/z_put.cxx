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

#include "../getargs.h"
#include "stdio.h"
#include "zenoh.hxx"
using namespace zenoh;

#ifdef ZENOHCXX_ZENOHC
const char *default_value = "Put from C++ zenoh-c!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-c-put";
#elif ZENOHCXX_ZENOHPICO
const char *default_value = "Put from C++ zenoh-pico!";
const char *default_keyexpr = "demo/example/zenoh-cpp-zenoh-pico-put";
#else
#error "Unknown zenoh backend"
#endif

int _main(int argc, char **argv) {
    const char *keyexpr = default_keyexpr;
    const char *value = default_value;
    const char *locator = nullptr;
    const char *configfile = nullptr;

    getargs(argc, argv, {}, {{"key expression", &keyexpr}, {"value", &value}, {"locator", &locator}}
#ifdef ZENOHCXX_ZENOHC
            ,
            {{"-c", {"config file", &configfile}}}
#endif
    );

    Config config;
#ifdef ZENOHCXX_ZENOHC
    if (configfile) {
        config = expect(config_from_file(configfile));
    }
#endif

    if (locator) {
#ifdef ZENOHCXX_ZENOHC
        auto locator_json_str_list = std::string("[\"") + locator + "\"]";
        if (!config.insert_json(Z_CONFIG_CONNECT_KEY, locator_json_str_list.c_str()))
#elif ZENOHCXX_ZENOHPICO
        if (!config.insert(Z_CONFIG_CONNECT_KEY, locator))
#else
#error "Unknown zenoh backend"
#endif
        {
            std::cout << "Invalid locator: " << locator << std::endl;
            std::cout << "Expected value in format: tcp/192.168.64.3:7447" << std::endl;
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = expect<Session>(open(std::move(config)));

    printf("Putting Data ('%s': '%s')...\n", keyexpr, value);
    PutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
#ifdef ZENOHCXX_ZENOHC
    std::map<std::string, std::string> amap;
    amap.insert(std::make_pair("serial_number", "123"));
    amap.insert(std::make_pair("coordinates", "48.7082,2.1498"));
    options.set_attachment(amap);
#endif
    if (!session.put(keyexpr, value, options)) {
        printf("Put failed...\n");
    }

    return 0;
}

int main(int argc, char **argv) {
    try {
        _main(argc, argv);
    } catch (ErrorMessage e) {
        std::cout << "Received an error :" << e.as_string_view() << "\n";
    }
}
