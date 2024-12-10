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
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "zenoh.hxx"

class CliArgParser {
    struct PositionalArg {
        std::string name;
        std::string description;
    };

    struct OptionalPositionalArg {
        std::string name;
        std::string description;
        std::string_view value;
    };

    struct NamedFlag {
        std::vector<std::string> opts;
        std::string description;
    };

    struct NamedValue {
        std::vector<std::string> opts;
        std::string name;
        std::string description;
        std::string_view value;
    };

    struct NamedValues {
        std::vector<std::string> opts;
        std::string name;
        std::string description;
    };

    using NamedArg = std::variant<NamedFlag, NamedValue, NamedValues>;

    int _argc;
    char** _argv;
    std::vector<PositionalArg> _required;
    std::vector<OptionalPositionalArg> _optional;
    std::vector<NamedArg> _named;

    static void print_opts(const std::vector<std::string>& opts, std::ostream& os) {
        for (size_t i = 0; i < opts.size(); i++) {
            if (opts[i].size() == 1) {
                os << '-' << opts[i];
            } else if (opts[i].size() > 1) {
                os << "--" << opts[i];
            }
            if (i + 1 != opts.size()) {
                os << ", ";
            }
        }
    }

    static void print_help_string(const NamedFlag& flag, std::ostream& os) {
        print_opts(flag.opts, os);
        os << "  " << flag.description;
    }

    static void print_help_string(const NamedValue& val, std::ostream& os) {
        print_opts(val.opts, os);
        os << " <" << val.name << ">";
        os << "  " << val.description;
        os << " [default: " << val.value << "]";
    }

    static void print_help_string(const NamedValues& vals, std::ostream& os) {
        print_opts(vals.opts, os);
        os << " <" << vals.name << ">";
        os << "  " << vals.description;
    }

    void print_help(std::ostream& os) const {
        os << "Usage: " << std::filesystem::path(_argv[0]).filename().string();
        if (!_named.empty()) {
            os << " [OPTIONS]";
        }

        for (const auto& p : _required) {
            os << " <" << p.name + ">";
        }
        for (const auto& p : _optional) {
            os << " [<" << p.name << ">]";
        }

        os << std::endl;
        if (!_required.empty() || !_optional.empty()) {
            os << "Arguments:" << std::endl;
        }
        for (const auto& r : _required) {
            os << "  <" << r.name << "> " << r.description << std::endl;
        }
        for (const auto& o : _optional) {
            os << "  [<" << o.name << ">] " << o.description << " [default: " << o.value << "]" << std::endl;
        }
        os << "Options:" << std::endl;
        for (const auto& n : _named) {
            os << "  ";
            std::visit([&os](const auto& arg) { CliArgParser::print_help_string(arg, os); }, n);
            os << std::endl;
        }
        std::cout << "  ";
        print_help_string(NamedFlag{{"h", "help"}, "Print help"}, os);
        os << std::endl;
    }

   public:
    class Result {
        struct ArgFlagValue {
            bool value;
        };
        struct ArgValue {
            std::string_view value;
        };
        struct ArgValues {
            std::vector<std::string_view> values;
        };
        using NamedArgValue = std::variant<ArgFlagValue, ArgValue, ArgValues>;

        std::vector<std::string_view> _positional;
        std::vector<std::string_view> _optional;
        std::vector<NamedArgValue> _named;
        std::unordered_map<std::string, size_t> _opt_to_named_idx;

        friend class CliArgParser;
        Result(std::vector<std::string_view>&& positional, std::vector<std::string_view>&& optional,
               std::vector<NamedArgValue>&& named, std::unordered_map<std::string, size_t>&& opt_to_named_idx)
            : _positional(std::move(positional)),
              _optional(std::move(optional)),
              _named(std::move(named)),
              _opt_to_named_idx(std::move(opt_to_named_idx)) {}

       public:
        std::string_view positional(size_t idx) const { return _positional[idx]; }
        std::string_view optional(size_t idx) const { return _optional[idx]; }

        bool flag(size_t idx) const { return std::get<ArgFlagValue>(_named[idx]).value; }
        std::string_view value(size_t idx) const { return std::get<ArgValue>(_named[idx]).value; }
        const std::vector<std::string_view>& values(size_t idx) const {
            return std::get<ArgValues>(_named[idx]).values;
        }

        bool flag(const std::string& opt) const { return flag(_opt_to_named_idx.at(opt)); }
        std::string_view value(const std::string& opt) const { return value(_opt_to_named_idx.at(opt)); }
        const std::vector<std::string_view>& values(const std::string& opt) const {
            return values(_opt_to_named_idx.at(opt));
        }
    };

    Result run() {
        // Show help if help option is passed or if no parameters are passed when some are required
        if ((_argc == 2 && (strcmp(_argv[1], "--help") == 0 || strcmp(_argv[1], "-h") == 0)) ||
            _argc == 1 && _required.size() > 0) {
            print_help(std::cout);
            exit(0);
        }

        std::unordered_map<std::string, size_t> opt_to_named_arg_index;
        for (size_t i = 0; i < _named.size(); i++) {
            const auto& opts = std::visit([](const auto& arg) { return arg.opts; }, _named[i]);
            for (const auto& o : opts) {
                opt_to_named_arg_index[o] = i;
            }
        }

        std::vector<std::string_view> required_out(_required.size());
        std::vector<std::string_view> optional_out(_optional.size());
        for (size_t i = 0; i < _optional.size(); i++) {
            optional_out[i] = _optional[i].value;
        }
        std::vector<Result::NamedArgValue> named_out;
        named_out.reserve(_named.size());
        for (size_t i = 0; i < _named.size(); i++) {
            if (const auto named_flag = std::get_if<NamedFlag>(&_named[i]); named_flag != nullptr) {
                named_out.push_back(Result::ArgFlagValue{false});
            } else if (const auto named_value = std::get_if<NamedValue>(&_named[i]); named_value != nullptr) {
                named_out.push_back(Result::ArgValue{named_value->value});
            } else if (const auto named_values = std::get_if<NamedValues>(&_named[i]); named_values != nullptr) {
                named_out.push_back(Result::ArgValues{});
            }
        }

        for (size_t i = 0; i < _required.size(); i++) {
            if (i + 1 >= _argc) {
                throw std::runtime_error(std::string("Missing required argument <") + _required[i].name + ">");
            } else {
                required_out[i] = _argv[i + 1];
            }
        }

        size_t current_arg = _required.size() + 1;
        for (size_t i = 0; i < _optional.size(); i++) {
            if (current_arg >= _argc) {
                return Result(std::move(required_out), std::move(optional_out), std::move(named_out),
                              std::move(opt_to_named_arg_index));
            } else if (_argv[current_arg][0] == '-') {  // start of named arguments
                break;
            } else {
                optional_out[i] = _argv[i + _required.size() + 1];
            }
            current_arg += 1;
        }

        while (current_arg < _argc) {
            std::string_view a = _argv[current_arg];
            if (a.size() < 2 || a[0] != '-' || (a.size() > 2 && a[1] != '-')) {
                throw std::runtime_error(std::string("Unexpected option: ") + _argv[current_arg]);
            }
            auto it = (a[1] == '-') ? opt_to_named_arg_index.find(std::string(a.substr(2)))
                                    : opt_to_named_arg_index.find(std::string(a.substr(1)));
            if (it == opt_to_named_arg_index.end()) {
                throw std::runtime_error(std::string("Unexpected option: ") + _argv[current_arg]);
            }
            size_t arg_index = it->second;
            current_arg++;
            const auto& named_arg = _named[arg_index];
            if (const auto named_flag = std::get_if<NamedFlag>(&named_arg); named_flag != nullptr) {
                std::get<Result::ArgFlagValue>(named_out[arg_index]).value = true;
            } else if (current_arg >= _argc) {
                throw std::runtime_error(std::string("Option: ") + it->first + " requires a value");
            } else if (const auto named_value = std::get_if<NamedValue>(&named_arg); named_value != nullptr) {
                std::get<Result::ArgValue>(named_out[arg_index]).value = _argv[current_arg++];
            } else if (const auto named_values = std::get_if<NamedValues>(&named_arg); named_values != nullptr) {
                std::get<Result::ArgValues>(named_out[arg_index]).values.push_back(_argv[current_arg++]);
            }
        }
        return Result(std::move(required_out), std::move(optional_out), std::move(named_out),
                      std::move(opt_to_named_arg_index));
    }

    CliArgParser(int argc, char** argv) : _argc(argc), _argv(argv){};

    CliArgParser& positional(std::string name, std::string description) {
        _required.push_back({std::move(name), std::move(description)});
        return *this;
    }

    CliArgParser& optional(std::string name, std::string description, std::string_view value) {
        _optional.push_back({std::move(name), std::move(description), value});
        return *this;
    }

    CliArgParser& named_flag(std::vector<std::string> opts, std::string description) {
        _named.push_back(NamedFlag{std::move(opts), std::move(description)});
        return *this;
    }

    CliArgParser& named_value(std::vector<std::string> opts, std::string name, std::string description,
                              std::string_view value) {
        _named.push_back(NamedValue{std::move(opts), std::move(name), std::move(description), value});
        return *this;
    }

    CliArgParser& named_values(std::vector<std::string> opts, std::string name, std::string description) {
        _named.push_back(NamedValues{std::move(opts), std::move(name), std::move(description)});
        return *this;
    }
};

class ConfigCliArgParser : public CliArgParser {
   public:
    ConfigCliArgParser(int argc, char** argv) : CliArgParser(argc, argv){};

    ConfigCliArgParser& positional(std::string name, std::string description) {
        CliArgParser::positional(std::move(name), std::move(description));
        return *this;
    }

    ConfigCliArgParser& optional(std::string name, std::string description, std::string_view value) {
        CliArgParser::optional(std::move(name), std::move(description), value);
        return *this;
    }

    ConfigCliArgParser& named_flag(std::vector<std::string> opts, std::string description) {
        CliArgParser::named_flag(std::move(opts), std::move(description));
        return *this;
    }

    ConfigCliArgParser& named_value(std::vector<std::string> opts, std::string name, std::string description,
                                    std::string_view value) {
        CliArgParser::named_value(std::move(opts), std::move(name), std::move(description), value);
        return *this;
    }

    ConfigCliArgParser& named_values(std::vector<std::string> opts, std::string name, std::string description) {
        CliArgParser::named_values(std::move(opts), std::move(name), std::move(description));
        return *this;
    }

    std::tuple<zenoh::Config, CliArgParser::Result> run() {
#ifdef ZENOHCXX_ZENOHC
        named_value({"c", "config"}, "CONFIG_FILE", "Configuration file", "");
        named_values({"e", "connect"}, "CONNECT", "Endpoints to connect to");
        named_values({"l", "listen"}, "LISTEN", "Endpoints to listen to");
#elif defined(ZENOHCXX_ZENOHPICO)
        named_value({"e", "connect"}, "CONNECT", "Endpoint to connect to", "");
        named_value({"l", "listen"}, "LISTEN", "Endpoint to listen to", "");
#endif

#ifdef ZENOHCXX_ZENOHC
        named_value({"m", "mode"}, "MODE", "Zenoh session mode (peer | client)", "peer");
#elif defined(ZENOHCXX_ZENOHPICO)
        named_value({"m", "mode"}, "MODE", "Zenoh session mode (peer | client)", "client");
#endif
        named_flag({"no-multicast-scouting"}, "Disable the multicast-based scouting mechanism");

        auto result = CliArgParser::run();

        zenoh::Config config = zenoh::Config::create_default();
        auto mode = result.value("m");

        if (mode != "peer" && mode != "client") {
            throw std::runtime_error("Mode can only be 'peer' or 'client'");
        }

#ifdef ZENOHCXX_ZENOHC
        auto config_file = result.value("c");
        if (!config_file.empty()) {
            config = zenoh::Config::from_file(std::string(config_file));
        }
        const auto& locators = result.values("l");
        if (!locators.empty()) {
            config.insert_json5(Z_CONFIG_LISTEN_KEY, to_json_array_string(locators));
        }
        const auto& endpoints = result.values("e");
        if (!endpoints.empty()) {
            config.insert_json5(Z_CONFIG_CONNECT_KEY, to_json_array_string(endpoints));
        }
        config.insert_json5(Z_CONFIG_MODE_KEY, std::string("\"") + mode.data() + "\"");
        if (result.flag("no-multicast-scouting")) {
            config.insert_json5(Z_CONFIG_MULTICAST_SCOUTING_KEY, "false");
        }
#elif defined(ZENOHCXX_ZENOHPICO)
        if (!mode.empty()) {
            config.insert(Z_CONFIG_MODE_KEY, mode.data());
            if (mode == "peer") {
                auto locator = result.value("l");
                if (locator.empty()) {
                    throw std::runtime_error(
                        "Zenoh-Pico in 'peer' mode requires providing a multicast group locator to listen to (-l "
                        "option), "
                        "e. g. 'udp/224.0.0.224:7447#iface=lo'");
                } else {
                    config.insert(Z_CONFIG_LISTEN_KEY, locator.data());
                }
            } else if (mode == "client") {
                auto endpoint = result.value("e");
                if (!endpoint.empty()) {
                    config.insert(Z_CONFIG_CONNECT_KEY, endpoint.data());
                }
            }
        }
        if (result.flag("no-multicast-scouting")) {
            config.insert(Z_CONFIG_MULTICAST_SCOUTING_KEY, "false");
        }
#endif
        return {std::move(config), std::move(result)};
    }

    static std::string to_json_array_string(const std::vector<std::string_view>& v) {
        if (v.empty()) {
            return "[]";
        }
        std::stringstream ss;
        ss << "[";
        ss << '"' << v[0] << '"';
        for (size_t i = 1; i < v.size(); i++) {
            ss << ',' << '"' << v[0] << '"';
        }
        ss << "]";
        return ss.str();
    }
};

inline zenoh::QueryTarget parse_query_target(std::string_view v) {
    if (v == "BEST_MATCHING") {
        return zenoh::QueryTarget::Z_QUERY_TARGET_BEST_MATCHING;
    } else if (v == "ALL") {
        return zenoh::QueryTarget::Z_QUERY_TARGET_ALL;
    } else if (v == "ALL_COMPLETE") {
        return zenoh::QueryTarget::Z_QUERY_TARGET_ALL_COMPLETE;
    }

    throw std::runtime_error(std::string("Unsupported QueryTarget: ") + std::string(v));
}

struct Selector {
    std::string key_expr;
    std::string parameters;
};

inline Selector parse_selector(std::string_view selector_string) {
    size_t pos = selector_string.find('?');
    if (pos == std::string::npos) {
        return Selector{std::string(selector_string), ""};
    } else {
        return Selector{std::string(selector_string.substr(0, pos)), std::string(selector_string.substr(pos + 1))};
    }
}

inline zenoh::Priority parse_priority(std::string_view arg) {
    int p = std::atoi(arg.data());
    if (p < zenoh::Priority::Z_PRIORITY_INTERACTIVE_HIGH || p > zenoh::Priority::Z_PRIORITY_BACKGROUND) {
        throw std::runtime_error(std::string("Unsupported Priority: ") + std::to_string(p));
    }
    return (zenoh::Priority)p;
}