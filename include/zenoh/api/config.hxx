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

#include <string>
#include <vector>

#include "base.hxx"

namespace zenoh {
/// A Zenoh Session config.
class Config : public Owned<::z_owned_config_t> {
    Config(zenoh::detail::null_object_t) : Owned(nullptr){};

   public:
    /// @name Constructors

    /// @brief Create a default configuration.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    static Config create_default(ZResult* err = nullptr) {
        Config c(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(::z_config_default(&c._0), err, std::string("Failed to create default configuration"));
        return c;
    }

#ifdef ZENOHCXX_ZENOHC
    /// @brief Create the configuration from the JSON file.
    /// @param path path to the config file (see <a
    /// href="https://zenoh.io/docs/manual/configuration/#configuration-files">config file documentation</a> for more
    /// information).
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return the ``Config`` object.
    /// @note Zenoh-c only.
    static Config from_file(const std::string& path, ZResult* err = nullptr) {
        Config c(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(::zc_config_from_file(&c._0, path.data()), err,
                             std::string("Failed to create config from: ").append(path));
        return c;
    }

    /// @brief Create the configuration from the JSON string.
    /// @param s config in JSON format.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return the ``Config`` object.
    /// @note Zenoh-c only.
    static Config from_str(const std::string& s, ZResult* err = nullptr) {
        Config c(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(::zc_config_from_str(&c._0, s.data()), err,
                             std::string("Failed to create config from: ").append(s));
        return c;
    }

    /// @brief Create a configuration by parsing a file with path stored in ZENOH_CONFIG environment variable.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return the ``Config`` object.
    /// @note Zenoh-pico only.
    static Config from_env(ZResult* err = nullptr) {
        Config c(zenoh::detail::null_object);
        __ZENOH_RESULT_CHECK(::zc_config_from_env(&c._0), err, "Failed to create config from environment variable");
        return c;
    }
#endif

    /// @name Methods

#ifdef ZENOHCXX_ZENOHC
    /// @brief Get config parameter by the string key.
    /// @param key the key.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return value of the config parameter in JSON format.
    /// @note Zenoh-c only.
    std::string get(std::string_view key, ZResult* err = nullptr) const {
        ::z_owned_string_t s;
        __ZENOH_RESULT_CHECK(::zc_config_get_from_substr(interop::as_loaned_c_ptr(*this), key.data(), key.size(), &s),
                             err, std::string("Failed to get config value for the key: ").append(key));
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Get the whole config as a JSON string.
    /// @return string with config in json format.
    /// @note Zenoh-c only.
    std::string to_string() const {
        ::z_owned_string_t s;
        ::zc_config_to_string(interop::as_loaned_c_ptr(*this), &s);
        std::string out = std::string(::z_string_data(::z_loan(s)), ::z_string_len(::z_loan(s)));
        ::z_drop(::z_move(s));
        return out;
    }

    /// @brief Insert a config parameter by the string key.
    /// @param key the key.
    /// @param value the JSON string value,
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return ``true`` if the parameter was inserted, false otherwise.
    /// @note Zenoh-c only.
    void insert_json5(const std::string& key, const std::string& value, ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(::zc_config_insert_json5(interop::as_loaned_c_ptr(*this), key.c_str(), value.c_str()), err,
                             std::string("Failed to insert '")
                                 .append(value)
                                 .append("' for the key '")
                                 .append(key)
                                 .append("' into config"));
    }
#endif
#ifdef ZENOHCXX_ZENOHPICO
    /// @brief Get config parameter by it's numeric ID.
    /// @param key the key.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return pointer to the null-terminated string value of the config parameter.
    /// @note Zenoh-pico only.
    const char* get(uint8_t key, ZResult* err = nullptr) const {
        const char* c = ::zp_config_get(interop::as_loaned_c_ptr(*this), key);
        __ZENOH_RESULT_CHECK((c == nullptr ? -1 : Z_OK), err,
                             std::string("Failed to get config value for the key: ").append(std::to_string(key)));
        return c;
    }

    /// @brief Insert a config parameter by it's numeric ID.
    /// @param key the key.
    /// @param value the null-terminated string value.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note Zenoh-pico only.
    void insert(uint8_t key, const char* value, ZResult* err = nullptr) {
        __ZENOH_RESULT_CHECK(zp_config_insert(interop::as_loaned_c_ptr(*this), key, value), err,
                             std::string("Failed to insert '")
                                 .append(value)
                                 .append("' for the key '")
                                 .append(std::to_string(key))
                                 .append("' into config"));
    }
#endif
};
}  // namespace zenoh
