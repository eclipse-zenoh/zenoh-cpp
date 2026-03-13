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

#pragma once

#if defined(Z_FEATURE_UNSTABLE_API)

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../zenohc.hxx"
#include "base.hxx"
#include "enums.hxx"
#include "id.hxx"
#include "interop.hxx"

namespace zenoh {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief Represents a data link to a remote zenoh node over a specific protocol.
class Link : public Owned<::z_owned_link_t> {
   public:
    /// @name Methods

    /// @brief Get the Zenoh ID of the remote node.
    /// @return `Id` of the remote node.
    Id get_zid() const { return interop::into_copyable_cpp_obj<Id>(::z_link_zid(interop::as_loaned_c_ptr(*this))); }

    /// @brief Get the source address of the link.
    /// @return source address string.
    std::string get_src() const {
        ::z_owned_string_t str_out;
        ::z_link_src(interop::as_loaned_c_ptr(*this), &str_out);
        std::string result(::z_string_data(::z_loan(str_out)), ::z_string_len(::z_loan(str_out)));
        ::z_drop(::z_move(str_out));
        return result;
    }

    /// @brief Get the destination address of the link.
    /// @return destination address string.
    std::string get_dst() const {
        ::z_owned_string_t str_out;
        ::z_link_dst(interop::as_loaned_c_ptr(*this), &str_out);
        std::string result(::z_string_data(::z_loan(str_out)), ::z_string_len(::z_loan(str_out)));
        ::z_drop(::z_move(str_out));
        return result;
    }

    /// @brief Get the group of the link.
    /// @return group string or `std::nullopt` if not available.
    std::optional<std::string> get_group() const {
        ::z_owned_string_t str_out;
        ::z_link_group(interop::as_loaned_c_ptr(*this), &str_out);
        std::string result(::z_string_data(::z_loan(str_out)), ::z_string_len(::z_loan(str_out)));
        ::z_drop(::z_move(str_out));
        if (result.empty()) return std::nullopt;
        return result;
    }

    /// @brief Get the authentication identifier of the link.
    /// @return authentication identifier string or `std::nullopt` if not available.
    std::optional<std::string> get_auth_identifier() const {
        ::z_owned_string_t str_out;
        ::z_link_auth_identifier(interop::as_loaned_c_ptr(*this), &str_out);
        std::string result(::z_string_data(::z_loan(str_out)), ::z_string_len(::z_loan(str_out)));
        ::z_drop(::z_move(str_out));
        if (result.empty()) return std::nullopt;
        return result;
    }

    /// @brief Get the MTU (Maximum Transmission Unit) of the link.
    /// @return MTU value.
    uint16_t get_mtu() const { return ::z_link_mtu(interop::as_loaned_c_ptr(*this)); }

    /// @brief Check if the link is streamed.
    /// @return `true` if the link is streamed, `false` otherwise.
    bool is_streamed() const { return ::z_link_is_streamed(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the network interfaces associated with this link.
    /// @return vector of interface names.
    std::vector<std::string> get_interfaces() const {
        ::z_owned_string_array_t interfaces_out;
        ::z_link_interfaces(interop::as_loaned_c_ptr(*this), &interfaces_out);
        auto loaned = ::z_loan(interfaces_out);
        std::vector<std::string> result;
        result.reserve(::z_string_array_len(loaned));
        for (size_t i = 0; i < ::z_string_array_len(loaned); i++) {
            auto s = ::z_string_array_get(loaned, i);
            result.emplace_back(::z_string_data(s), ::z_string_len(s));
        }
        ::z_drop(::z_move(interfaces_out));
        return result;
    }

    /// @brief Get the priority range supported by this link.
    /// @return pair of (min, max) priority values or `std::nullopt` if not available.
    std::optional<std::pair<uint8_t, uint8_t>> get_priorities() const {
        uint8_t min_val, max_val;
        if (::z_link_priorities(interop::as_loaned_c_ptr(*this), &min_val, &max_val)) {
            return std::make_pair(min_val, max_val);
        }
        return std::nullopt;
    }

    /// @brief Get the reliability of this link.
    /// @return reliability value or `std::nullopt` if not available.
    std::optional<Reliability> get_reliability() const {
        Reliability reliability;
        if (::z_link_reliability(interop::as_loaned_c_ptr(*this), &reliability)) {
            return reliability;
        }
        return std::nullopt;
    }

    /// @brief Copy constructor.
    Link(const Link& other) : Owned(nullptr) { ::z_link_clone(&this->_0, interop::as_loaned_c_ptr(other)); }

    /// @brief Move constructor.
    Link(Link&& other) = default;

    /// @name Operators

    /// @brief Assignment operator.
    Link& operator=(const Link& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_link_clone(&this->_0, interop::as_loaned_c_ptr(other));
        }
        return *this;
    }

    /// @brief Move assignment operator.
    Link& operator=(Link&& other) = default;
};

}  // namespace zenoh
#endif
