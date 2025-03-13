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
#include <string_view>
#include <vector>

#include "../zenohc.hxx"
#include "base.hxx"
#include "enums.hxx"
#include "id.hxx"
#include "interop.hxx"

namespace zenoh {
/// ``Hello`` message returned by a zenoh entity as a reply to a "scout"
/// message.
class Hello : public Owned<::z_owned_hello_t> {
   public:
    /// @name Methods

    /// @brief Get ``Id`` of the entity.
    /// @return ``Id`` of the entity.
    Id get_id() const { return interop::into_copyable_cpp_obj<Id>(::z_hello_zid(interop::as_loaned_c_ptr(*this))); };

    /// @brief Get the type of the entity.
    /// @return ``zenoh::WhatAmI`` of the entity.
    WhatAmI get_whatami() const { return ::z_hello_whatami(interop::as_loaned_c_ptr(*this)); }

    /// @brief Get the array of locators of the entity.
    /// @return the array of locators of the entity.
    std::vector<std::string_view> get_locators() const {
#ifdef ZENOHCXX_ZENOHC
        ::z_owned_string_array_t out;
        ::z_hello_locators(interop::as_loaned_c_ptr(*this), &out);
        auto out_loaned = ::z_loan(out);
#else
        auto out_loaned = ::zp_hello_locators(interop::as_loaned_c_ptr(*this));
#endif
        std::vector<std::string_view> locators(::z_string_array_len(out_loaned));
        for (size_t i = 0; i < ::z_string_array_len(out_loaned); i++) {
            auto s = ::z_string_array_get(out_loaned, i);
            locators[i] = std::string_view(reinterpret_cast<const char*>(::z_string_data(s)), ::z_string_len(s));
        }
#ifdef ZENOHCXX_ZENOHC
        z_drop(z_move(out));
#endif
        return locators;
    }

    /// @brief Copy constructor.
    Hello(const Hello& other) : Owned(nullptr) { ::z_hello_clone(&this->_0, interop::as_loaned_c_ptr(other)); };

    /// @brief Move constructor.
    Hello(Hello&& other) = default;

    /// @name Operators

    /// @brief Assignment operator.
    Hello& operator=(const Hello& other) {
        if (this != &other) {
            ::z_drop(z_move(this->_0));
            ::z_hello_clone(&this->_0, interop::as_loaned_c_ptr(other));
        }
        return *this;
    };

    /// @brief Move assignment operator.
    Hello& operator=(Hello&& other) = default;
};
}  // namespace zenoh
