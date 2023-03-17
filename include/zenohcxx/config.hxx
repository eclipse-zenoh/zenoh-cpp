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

// Do not add '#pragma once' and '#include` statements here
// as this file is included multiple times into different namespaces

class ScoutingConfig;

class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
#ifdef __ZENOHCXX_ZENOHC
    Str get(const char* key) const { return Str(::zc_config_get(::z_config_loan(&_0), key)); }
    Str to_string() const { return Str(::zc_config_to_string(::z_config_loan(&_0))); }
    bool insert_json(const char* key, const char* value) {
        return ::zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    const char* get(uint8_t key) const { return ::zp_config_get(::z_config_loan(&_0), key); }
#endif
    ScoutingConfig create_scouting_config();
};

#ifdef __ZENOHCXX_ZENOHC
Config config_peer() { return Config(::z_config_peer()); }

std::variant<Config, ErrorMessage> config_from_file(const char* path) {
    Config config(::zc_config_from_file(path));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from file";
    }
}

std::variant<Config, ErrorMessage> config_from_str(const char* s) {
    Config config(::zc_config_from_str(s));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from string";
    }
}

std::variant<Config, ErrorMessage> config_client(const StrArrayView& peers) {
    Config config(::z_config_client(peers.val, peers.len));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from list of peers";
    }
}

std::variant<Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers) {
    std::vector<const char*> v(peers);
    return config_client(v);
}
#endif

class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
    ScoutingConfig(Config& config) : Owned(std::move(ScoutingConfig(config))) {}
};

inline ScoutingConfig Config::create_scouting_config() {
    return ScoutingConfig(::z_scouting_config_from(::z_loan(_0)));
}
