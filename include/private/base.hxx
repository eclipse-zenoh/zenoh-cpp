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
    Copyable(const InnerType& v) : InnerType(v) {}
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
            ::z_null(*pv);
        } else
            ::z_null(_0);
    }
    /// Move constructor from other object
    Owned(Owned&& v) : Owned(&v._0) {}
    /// Move assignment from other object
    Owned& operator=(Owned&& v) {
        if (this != &v) {
            ::z_drop(::z_move(this->_0));
            _0 = v._0;
            ::z_null(v._0);
        }
        return this;
    }
    /// Destructor drops owned value using z_drop from zenoh API
    ~Owned() { ::z_drop(::z_move(_0)); }

    /// @name Methods

    /// Take out zenoh structure and leave owned object in a null state.
    OwnedType take() && {
        auto r = _0;
        ::z_null(_0);
        return r;
    }
    /// Check object validity uzing zenoh API
    explicit operator bool() const { return ::z_check(_0); }

    explicit operator const LoanedType*() const { return loan(); }
    explicit operator LoanedType*() { return loan(); }

protected:
    OwnedType _0;

    const LoanedType* loan() const { return ::z_loan(_0); }
    LoanedType* loan() { return ::z_loan_mut(_0); }
};

template<class F, class R, class ...Args>
class Closure {
    typename std::conditional<std::is_lvalue_reference<F>, F, std::remove_reference<F>>::type func;
public:
    Closure(F&& f)
        : func(std::forward<F>(f))
    {}

    R operator()(Args... args) {
        return f(std::forward<Args>(args)...);
    }
};

/// @brief Base type for C++ wrappers of Zenoh closures with const pointer parameter
/// @tparam ZC_CLOSURE_TYPE - zenoh-c closure type ``::z_owned_closure_XXX_t``
/// @tparam ZC_PARAM - zenoh-c parameter type which is passed to closure ``::z_owned_XXX_t``
/// @tparam ZCPP_PARAM - zenoh-cpp parameter type which wraps zenoh-c parameter type (e.g. ``Reply`` for
/// ``::z_owned_reply_t``)
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<ZC_PARAM, ZCPP_PARAM> && sizeof(ZC_PARAM) == sizeof(ZCPP_PARAM),
                                    bool> = true>
class ClosureConstRefParam : public Owned<ZC_CLOSURE_TYPE> {
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    /// @name Constructors

    /// @brief Construct uninitialized closure
    ClosureConstRefParam() : Owned<ZC_CLOSURE_TYPE>(nullptr) {}

    /// @brief Construct closure from the data handler: any object with ``operator()(const ZCPP_PARAM&)`` defined
    /// @param on_call data handler - any object with ``operator()(const ZCPP_PARAM&)`` defined
    template <typename C>
    ClosureConstRefParam(C&& on_call) : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<C>(on_call), []() {})) {}

    /// @brief Construct closure from the data handler and the drop handler
    /// @param on_call data handler - any object with operator()(const ZCPP_PARAM&) defined
    /// @param on_drop drop handler - any object with operator()() defined
    ///
    ///
    /// Drop handler is convenient when it's necessary to catch dropping of the closure costructed from function
    /// pointer, object lvalue reference or lambda. If the closure holds the user's object, the additional drop handler
    /// is probably excessive. The cleanup in this case may be done in the object's destructor.
    template <typename C, typename D>
    ClosureConstRefParam(C&& on_call, D&& on_drop)
        : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<C>(on_call), std::forward<D>(on_drop))) {}

    /// @name Methods

    /// @brief Check if closure is valid. Closure is valid if it can be called, i.e. ``call`` is not ``nullptr``.
    /// ``drop`` operation is optional
    /// @return true if closure is valid
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    /// @brief Call closure with pointer to C parameter (``::z_owned_XXX_t``)
    /// @param v - pointer to C parameter
    /// @return value returned by closure
    ZC_RETVAL call(const ZC_PARAM* v) {
        if constexpr (std::is_same_v<ZC_RETVAL, void>) {
            if (check()) Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context);
        } else {
            return check() ? Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context) : ZC_RETVAL{};
        }
    }

    /// @brief Call closure with const reference to C++ parameter (wrapper for ``::z_owned_XXX_t``)
    /// @param v - const reference to C++ parameter
    ZC_RETVAL operator()(const ZCPP_PARAM& v) { return call(&(static_cast<const ZC_PARAM&>(v))); }

   private:
    template <typename C, typename D>
    ZC_CLOSURE_TYPE wrap_call(C&& on_call, D&& on_drop) {
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_call)>, C&, C> SC;
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_drop)>, D&, D> SD;
        // std::cout << "SC = " << type_name<SC>() << " SD = " << type_name<SD>() << std::endl;
        auto context = new std::pair<SC, SD>{std::forward<C>(on_call), std::forward<D>(on_drop)};
        auto call = [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            return pair->first(static_cast<const ZCPP_PARAM&>(*pvalue));
        };
        auto drop = [](void* ctx) {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            pair->second();
            delete pair;
        };
        return {context, call, drop};
    }
};

/// @brief Base type for C++ wrappers of Zenoh closures with owned parameter
/// @tparam ZC_CLOSURE_TYPE - zenoh-c closure type ``::z_owned_closure_XXX_t``
/// @tparam ZC_PARAM - zenoh-c parameter type which is passed to closure ``::z_XXX_t``
/// @tparam ZCPP_PARAM - zenoh-cpp parameter type which wraps zenoh-c parameter type (e.g. ``Sample`` for
/// ``::z_sample_t``)
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<
              std::is_base_of_v<Owned<ZC_PARAM>, ZCPP_PARAM> && sizeof(ZC_PARAM) == sizeof(ZCPP_PARAM), bool> = true>

class ClosureMoveParam : public Owned<ZC_CLOSURE_TYPE> {
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    /// @name Constructors

    /// @brief Construct uninitialized closure
    ClosureMoveParam() : Owned<ZC_CLOSURE_TYPE>(nullptr) {}

    /// @brief Construct closure from the data handler: any object with ``operator()(ZCPP_PARAM&)`` or
    /// ``operator()(ZCPP_PARAM&&)`` defined
    /// @param on_call data handler - any object with ``operator()(ZCPP_PARAM&)`` or ``operator()(ZCPP_PARAM&&)``
    template <typename T>
    ClosureMoveParam(T&& on_call) : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(on_call), []() {})) {}

    /// @brief Construct closure from the data handler and the drop handler
    /// @param on_call data handler - any object with ``operator()(const ZCPP_PARAM&)`` or
    /// ``operator()(ZCPP_PARAM&&)`` defined
    /// @param on_drop drop handler - any object with ``operator()()`` defined
    ///
    ///
    /// Drop handler is convenient when it's necessary to catch dropping of the closure costructed from function
    /// pointer, object lvalue reference or lambda. If the closure holds the user's object, the additional drop handler
    /// is probably excessive. The cleanup in this case may be done in the object's destructor.
    template <typename T, typename D>
    ClosureMoveParam(T&& on_call, D&& on_drop)
        : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(on_call), std::forward<D>(on_drop))) {}

    /// @brief Check if closure is valid. Closure is valid if it can be called, i.e. ``call`` is not ``nullptr``.
    /// ``drop`` operation is optional
    /// @return true if closure is valid
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    /// @brief Call closure with pointer to C parameter (``::z_XXX_t``)
    /// @param v - pointer to C parameter
    /// @return value returned by closure
    ZC_RETVAL call(ZC_PARAM* v) {
        if constexpr (std::is_same_v<ZC_RETVAL, void>) {
            if (check()) Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context);
        } else {
            return check() ? Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context) : ZC_RETVAL{};
        }
    }

    /// @brief Call closure with rvalue reference to C++ parameter
    /// @param v - rvalue reference to C++ parameter
    /// @return value returned by closure
    ZC_RETVAL operator()(ZCPP_PARAM&& v) { return call((&static_cast<ZC_PARAM&>(v))); }

    /// @brief Call closure with lvalue reference to C++ parameter
    /// @param v - lvalue reference to C++ parameter
    /// @return value returned by closure
    ZC_RETVAL operator()(ZCPP_PARAM& v) { return call((&static_cast<ZC_PARAM&>(v))); }

   private:
    template <typename C, typename D>
    ZC_CLOSURE_TYPE wrap_call(C&& on_call, D&& on_drop) {
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_call)>, C&, C> SC;
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_drop)>, D&, D> SD;
        auto context = new std::pair<SC, SD>{std::forward<C>(on_call), std::forward<D>(on_drop)};
        auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            return pair->first(std::move(reinterpret_cast<ZCPP_PARAM&>(*pvalue)));
        };
        auto drop = [](void* ctx) {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            pair->second();
            delete pair;
        };
        return {context, call, drop};
    }
};

}  // namespace zenoh
