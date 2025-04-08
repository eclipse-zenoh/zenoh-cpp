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

#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../detail/availability_checks.hxx"
#include "../zenohc.hxx"

namespace zenoh {

/// @brief Error code returned by Zenoh API
typedef ::z_result_t ZResult;

/// @brief Zenoh-specific Exception
class ZException : public std::runtime_error {
   public:
    ZResult e;
    ZException(const std::string& message, ZResult err)
        : std::runtime_error(message + "(Error code: " + std::to_string(err) + " )"), e(err) {}
};

#define __ZENOH_RESULT_CHECK(err, err_ptr, message)        \
    if (err_ptr == nullptr) {                              \
        ZResult __ze = static_cast<ZResult>(err);          \
        if (__ze != Z_OK) throw ZException(message, __ze); \
    } else {                                               \
        *err_ptr = static_cast<ZResult>(err);              \
    }

//
// Template base classes implementing common functionality
//

/// Base type for C++ wrappers of Zenoh copyable structures, like GetOptions, PutOptions, etc.
/// @tparam ZC_COPYABLE_TYPE - zenoh-c structure type ::z_XXX_t
template <typename ZC_COPYABLE_TYPE>
class Copyable {
   protected:
    typedef ZC_COPYABLE_TYPE InnerType;
    InnerType _0;

    InnerType& inner() { return this->_0; }
    const InnerType& inner() const { return this->_0; }
    explicit Copyable(const InnerType& v) : _0(v) {}
};

/// Base type for C++ wrappers of Zenoh owned structures
/// @tparam ZC_OWNED_TYPE - zenoh-c owned type ::z_owned_XXX_t
template <typename ZC_OWNED_TYPE>
class Owned {
   protected:
    typedef ZC_OWNED_TYPE OwnedType;

   protected:
    /// Move constructor.
    Owned(Owned&& v) : Owned(nullptr) { *this = std::move(v); }
    /// Move assignment.
    Owned& operator=(Owned&& v) {
        if (this != &v) {
            ::z_drop(::z_move(this->_0));
            if constexpr (detail::is_take_from_loaned_available_v<OwnedType>) {
                if (::z_internal_check(v._0)) {
                    ::z_take_from_loaned(&this->_0, ::z_loan_mut(v._0));
                } else {
                    ::z_internal_null(&this->_0);
                }
            } else {
                z_take(&this->_0, ::z_move(v._0));
            }
        }
        return *this;
    }
    /// Destructor drops owned value using z_drop from zenoh API
    ~Owned() { ::z_drop(::z_move(_0)); }

    OwnedType _0;

    explicit Owned(OwnedType* pv) {
        if (pv != nullptr) {
            z_take(&this->_0, ::z_move(*pv));
        } else {
            ::z_internal_null(&this->_0);
        }
    }
};

}  // namespace zenoh
