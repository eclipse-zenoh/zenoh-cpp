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

#include "zenohc.hxx"
#include <stdexcept>
#include <string>

namespace zenoh {

namespace detail {
template <typename T, typename = void>
struct is_loan_available : std::false_type {};

template <typename T>
struct is_loan_available<T, std::void_t<decltype(::z_loan(std::declval<const T&>())) >> : std::true_type {};

template< class T> inline constexpr bool is_loan_available_v = is_loan_available<T>::value;


template <typename T, typename = void>
struct is_loan_mut_available : std::false_type {};

template <typename T>
struct is_loan_mut_available<T, std::void_t<decltype(::z_loan_mut(std::declval<T&>())) >> : std::true_type {};

template< class T> inline constexpr bool is_loan_mut_available_v = is_loan_mut_available<T>::value;

}

/// Error code returned by Zenoh API
typedef ::z_error_t ZError;

/// @brief Zenoh-specific Exception
class ZException : public std::runtime_error {
public:
    ZError e;
    ZException(const std::string& message, ZError err)
        :std::runtime_error(message + "(Error code: " + std::to_string(err) + " )"), e(err) {
    }
};

#define __ZENOH_ERROR_CHECK(err, err_ptr, message)         \
    if (err_ptr == nullptr) {                              \
        ZError __ze = err;                                 \
        if (__ze != Z_OK) throw ZException(message, __ze); \
    } else {                                               \
        *err_ptr = err;                                    \
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
public:
    /// @name Constructors
    /// Construct from wrapped zenoh-c / zenoh-pico structure
    Copyable(const InnerType& v) : _0(v) {}
    explicit operator const ZC_COPYABLE_TYPE&() const { return inner(); }
    explicit operator ZC_COPYABLE_TYPE&() { return inner(); }
};

/// Base type for C++ wrappers of Zenoh owned structures
/// @tparam ZC_OWNED_TYPE - zenoh-c owned type ::z_owned_XXX_t
template <typename ZC_OWNED_TYPE>
class Owned {
protected:
    typedef typename z_owned_to_loaned_type_t<ZC_OWNED_TYPE>::type LoanedType;
    typedef ZC_OWNED_TYPE OwnedType;
public:
    /// @name Constructors
    /// @brief Construct from owned zenoh-c struct.
    /// @param pv Pointer to valid owned zenoh-c struct. The ownership is transferred
    /// to the constructed object.
    Owned(OwnedType* pv) {
        if (pv) {
            _0 = *pv;
            ::z_null(pv);
        } else
            ::z_null(&this->_0);
    }
    /// Move constructor from other object
    Owned(Owned&& v) : Owned(&v._0) {}
    /// Move assignment from other object
    Owned& operator=(Owned&& v) {
        if (this != &v) {
            ::z_drop(::z_move(this->_0));
            _0 = v._0;
            ::z_null(&v._0);
        }
        return *this;
    }
    /// Destructor drops owned value using z_drop from zenoh API
    ~Owned() { ::z_drop(::z_move(_0)); }

    /// @name Methods

    /// Take out zenoh structure and leave owned object in a null state.
    OwnedType take() && {
        auto r = this->_0;
        ::z_null(&this->_0);
        return r;
    }
    /// Check object validity uzing zenoh API
    explicit operator bool() const { return ::z_check(_0); }

protected:
    OwnedType _0;

    template<class T = const LoanedType*>
    typename std::enable_if<detail::is_loan_available_v<ZC_OWNED_TYPE>, T>::type
    loan() const { return ::z_loan(_0); }

    template<class T = LoanedType*>
    typename std::enable_if<detail::is_loan_mut_available_v<ZC_OWNED_TYPE>, T>::type
    loan() { return ::z_loan_mut(_0); }
};

template<class F, class R, class ...Args>
class Closure {
    typename std::conditional_t<std::is_lvalue_reference_v<F>, F, std::remove_reference_t<F>> func;
public:
    Closure(F&& f)
        : func(std::forward<F>(f))
    {}

    R operator()(Args... args) {
        return func(std::forward<Args>(args)...);
    }
};

namespace detail {
template<class F, class V>
struct AutoClosure: std::false_type {};

template<class F, class R, class ...Args>
struct AutoClosure<F, std::function<R(Args...)>> : std::true_type {
    typename std::conditional_t<std::is_lvalue_reference_v<F>, F, std::remove_reference_t<F>> func;
public:
    AutoClosure(F&& f)
        : func(std::forward<F>(f))
    {}

    R operator()(Args... args) {
        return func(std::forward<Args>(args)...);
    }
};

}

template<class F, class D>
class ScopedClosure : public detail::AutoClosure<F, decltype(std::function(std::declval<F>()))> {
    typename std::conditional_t<std::is_lvalue_reference_v<D>, D, std::remove_reference_t<D>> drop;
    typedef detail::AutoClosure<F, decltype(std::function(std::declval<F>()))> Base;
public:
    ScopedClosure(F&& f, D&& d)
        : Base(std::forward<F>(f)), drop(std::forward<D>(d))
    {}

    ~ScopedClosure() {
        drop();
    }
};

template<class F, class D>
auto make_scoped_closure(F&& f, D&& d) {
    return ScopedClosure<F, D>(std::forward<F>(f), std::forward<D>(d));
}

}