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


#pragma once

#include "base.hxx"
#include <string>
#include <vector>




namespace zenoh {
/// A Zenoh Session config
class Config : public Owned<::z_owned_config_t> {
public:
    using Owned::Owned;

    /// @name Constructors

    /// @brief Create a default configuration
    Config() : Owned(nullptr) {
        ::z_config_default(&this->_0);
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get config parameter by the string key
    /// @param key the key
    /// @return value of the config parameter
    /// @note zenoh-c only
    std::string get(std::string_view key, ZError* err = nullptr) const {
        ::z_owned_string_t s;
        __ZENOH_ERROR_CHECK(
            ::zc_config_get_from_substring(this->loan(), key.data(), key.size(), &s),
            err,
            std::string("Failed to get config value for the key: ").append(key)
        );
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Get the whole config as a JSON string
    /// @return the JSON string in ``Str``
    /// @note zenoh-c only
    std::string to_string() const { 
        ::z_owned_string_t s;
        ::zc_config_to_string(this->loan(), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out; 
    }

    /// @brief Insert a config parameter by the string key
    /// @param key the key
    /// @param value the JSON string value
    /// @return true if the parameter was inserted
    /// @note zenoh-c only
    bool insert_json(const std::string& key, const std::string& value) { 
        return ::zc_config_insert_json(loan(), key.c_str(), value.c_str()) == 0; 
    }
#endif
#ifdef ZENOHCXX_ZENOHPICO
    /// @brief Get config parameter by it's numeric ID
    /// @param key the key
    /// @return pointer to the null-terminated string value of the config parameter
    /// @note zenoh-pico only
    const char* get(uint8_t key) const { return ::zp_config_get(loan(), key); }

    /// @brief Insert a config parameter by it's numeric ID
    /// @param key the key
    /// @param value the null-terminated string value
    /// @return true if the parameter was inserted
    /// @note zenoh-pico only
    bool insert(uint8_t key, const char* value);

    /// @brief Insert a config parameter by it's numeric ID
    /// @param key the key
    /// @param value the null-terminated string value
    /// @param error the error code
    /// @return true if the parameter was inserted
    /// @note zenoh-pico only
    bool insert(uint8_t key, const char* value, ErrNo& error);
#endif

#ifdef ZENOHCXX_ZENOHC
    /// @brief Create the default configuration for "peer" mode
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config peer() {
        Config c(nullptr) ;
        ::z_config_peer(&c._0);
        return c;
    }

    /// @brief Create the configuration from the JSON file
    /// @param path path to the file
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config from_file(const std::string& path, ZError* err = nullptr) {
        Config c(nullptr) ;
        __ZENOH_ERROR_CHECK(
            ::zc_config_from_file(&c._0, path.data()),
            err,
            std::string("Failed to create config from: ").append(path)
        );
        return c;
    }

    /// @brief Create the configuration from the JSON string
    /// @param s the JSON string
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config from_str(const std::string& s, ZError* err = nullptr) {
        Config c(nullptr) ;
        __ZENOH_ERROR_CHECK(
            ::zc_config_from_str(&c._0, s.data()),
            err,
            std::string("Failed to create config from: ").append(s)
        );
        return c;
    }
    /// @brief Create the configuration for "client" mode
    /// @param peers the array of peer endpoints
    /// @return the ``Config`` object
    /// @note zenoh-c only
    static Config client(const std::vector<std::string>& peers, ZError* err = nullptr) {
        Config c(nullptr) ;
        std::vector<const char*> p;
        p.reserve(peers.size());
        for (const auto& peer: peers) {
            p.push_back(peer.c_str());
        }
        __ZENOH_ERROR_CHECK(
            ::z_config_client(&c._0, p.data(), p.size()),
            err,
            "Failed to create client config"
        );
        return c;
    }
    #endif
};
}