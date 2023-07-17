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

#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "assert.h"
#include "string.h"

namespace zenohcxx {

//
// Template base classes implementing common functionality
//

//
// Base type for C++ wrappers of Zenoh copyable structures, like GetOptions, PutOptions, etc.
//
template <typename ZC_COPYABLE_TYPE>
struct Copyable : public ZC_COPYABLE_TYPE {
    Copyable() = delete;  // May be overloaded in derived structs with corresponding z_XXX_default function
    Copyable(const Copyable& v) { *this = v; }
    Copyable(ZC_COPYABLE_TYPE v) : ZC_COPYABLE_TYPE(v) {}
};

//
// Base type for C++ wrappers of Zenoh owned structures, copying not allowed
//
template <typename ZC_OWNED_TYPE>
class Owned {
   public:
    // Default constructor is deleted by default, derived classes may override it to create default valid object.
    // It's supposed that default constructor never creates null object, this should be done explicitly with constructor
    // from nullptr
    Owned() = delete;
    // Copying is not allowed, owned object have ownership of it's value
    Owned& operator=(const Owned& v) = delete;
    Owned(const Owned& v) = delete;
    // Creating from pointer to value is allowed, ownership is taken and value is made null
    // Also explicit creation of null owned object is allowed if nullptr is passed
    Owned(ZC_OWNED_TYPE* pv) {
        if (pv) {
            _0 = *pv;
            ::z_null(*pv);
        } else
            ::z_null(_0);
    }
    // Move constructor from wrapped value
    Owned(ZC_OWNED_TYPE&& v) : _0(v) { ::z_null(v); }
    // Move constructor from other object
    Owned(Owned&& v) : Owned(std::move(v._0)) {}
    // Move assignment from other object
    Owned&& operator=(Owned&& v) {
        if (this != &v) {
            drop();
            _0 = v._0;
            ::z_null(v._0);
        }
        return std::move(*this);
    }
    // Destructor drops owned value using z_drop from zenoh API
    ~Owned() { ::z_drop(&_0); }
    // Explicit drop. Making value null is zenoh API job
    void drop() { ::z_drop(&_0); }
    // Take zenoh structure and leave Owned object null
    ZC_OWNED_TYPE take() {
        auto r = _0;
        ::z_null(_0);
        return r;
    }
    // Replace value with zenoh structure, dropping old value
    void put(ZC_OWNED_TYPE& v) {
        ::z_drop(&_0);
        _0 = v;
        ::z_null(v);
    }
    // Get direct access to wrapped zenoh structure
    explicit operator ZC_OWNED_TYPE&() { return _0; }
    explicit operator const ZC_OWNED_TYPE&() const { return _0; }
    // Check object validity uzing zenoh API
    bool check() const { return ::z_check(_0); }

   protected:
    ZC_OWNED_TYPE _0;
};

//
// Base type for C++ wrappers for Zenoh closures which doesn't take ownership of passed value
// It expects that
// - ZCPP_PARAM type is derived from ZC_PARAM
//
// All zenoh types wrapped with 'ClosureConstPtrParam' are defined this way:
//
// typedef struct ZC_CLOSURE_TYPE {
//   void *context;
//   void (*call)(const struct ZC_PARAM*, void*);
//   void (*drop)(void*);
// } ZC_CLOSURE_TYPE;
//
// `ClosureConstPtrParam` can be constructed from the following objects:
//
// - function pointer of type `void (func*)(const ZCPP_PARAM&)`
// 
//   Example:
// 
//   void on_query(const Query&) { ... }
//   ...
//   session.declare_queryable("foo/bar", on_query);
//
// - any object which can be called with `operator()(const ZCPP_PARAM&)`, e.g. lambda,
//   passed my move reference. In this case `ClosureConstPtrParam` will take ownership
//   of the object and will call it with `operator()(const ZCPP_PARAM&)` when `call` is called
//   and will drop it when `drop` is called.
//
//   Example:
//
//   session.declare_queryable("foo/bar", [](const Query&) { ... });
//
//   or
//  
//   struct OnQuery {
//     void operator()(const Query&) { ... }
//     ~OnQuery() { ... }
//   };  
//
//   OnQuery on_query;
//   session.declare_queryable("foo/bar", std::move(on_query));
//
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<ZC_PARAM, ZCPP_PARAM> && sizeof(ZC_PARAM) == sizeof(ZCPP_PARAM),
                                    bool> = true>
class ClosureConstPtrParam : public Owned<ZC_CLOSURE_TYPE> {
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    ZC_RETVAL call(const ZC_PARAM* v) { return Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context); }

    ZC_RETVAL operator()(const ZCPP_PARAM* v) { return call(v); }

    // Template constructors

    // Called with pointer to function accepting const ZCPP_PARAM&
    template <typename T, typename std::enable_if_t<std::is_function_v<T>, bool> = true>
    ClosureConstPtrParam(T& func) : Owned<ZC_CLOSURE_TYPE>(wrap_func(func)) {}

    // Called with rvalue reference to object with operator()(const ZCPP_PARAM&) defined
    template <typename T>
    ClosureConstPtrParam(T&& obj)
        : Owned<ZC_CLOSURE_TYPE>(wrap_forward(std::forward<std::remove_reference_t<T>>(obj))) {}

   private:
    template <typename T>
    ZC_CLOSURE_TYPE wrap_func(T& func) {
        // It's not allowed to cast pointer to function to void* and back, see detailed explanations here: 
        // https://stackoverflow.com/questions/36645660/why-cant-i-cast-a-function-pointer-to-void
        // So 'func' can't be directly stored in closure context field, so we wrap it in std::function
        typedef std::function<T> CONTEXT_TYPE;
        return {
            new CONTEXT_TYPE(func),
            [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
                return static_cast<CONTEXT_TYPE*>(ctx)->operator()(*static_cast<const ZCPP_PARAM*>(pvalue));
            },
            [](void* ctx) {
                // TODO: call destructor function if provided (make CONTEXT_TYPE std::pair instead)
                delete static_cast<CONTEXT_TYPE*>(ctx);
            }
        };
    }

    template <typename T>
    ZC_CLOSURE_TYPE wrap_forward(T&& obj) {
        typedef std::remove_reference_t<T> CONTEXT_TYPE;
        return {
            new CONTEXT_TYPE(std::forward<CONTEXT_TYPE>(obj)),
            [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
                return static_cast<CONTEXT_TYPE*>(ctx)->operator()(*static_cast<const ZCPP_PARAM*>(pvalue));
            },
            [](void* ctx) {
                delete static_cast<CONTEXT_TYPE*>(ctx);
            }
        };
    }
};

//
// Base type for C++ wrappers for Zenoh closures which takes ownership of passed value
// It expects that
// - ZCPP_PARAM is derived from Owned<ZC_PARAM>
// - user's LAMBDA can be invoked with ZCPP_PARAM, ZCPP_PARAM& or ZCPP_PARAM&&
// - user's LAMBDA is called by zenoh with empty ZCPP_PARAM (when .check()==false) to notify that closure is being
// destructed
//
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<Owned<ZC_PARAM>, ZCPP_PARAM>, bool> = true>

class ClosureMoveParam : public Owned<ZC_CLOSURE_TYPE> {
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    ZC_RETVAL call(ZC_PARAM* v) { return Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context); }

    ZC_RETVAL operator()(ZCPP_PARAM& v) { return call(&(static_cast<ZC_PARAM&>(v))); }
    ZC_RETVAL operator()(ZCPP_PARAM&& v) {
        ZCPP_PARAM take(std::move(v));
        return call(&(static_cast<ZC_PARAM&>(take)));
    }

    template <typename T, typename std::enable_if_t<!std::is_function_v<T>, bool> = true>
    ClosureMoveParam(T& obj) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda<false, true>(obj)) {}

    template <typename T, typename std::enable_if_t<std::is_function_v<T>, bool> = true>
    ClosureMoveParam(T& func) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda<true, false>(func)) {}

    template <typename T>
    ClosureMoveParam(T&& obj)
        : Owned<ZC_CLOSURE_TYPE>(wrap_lambda<false, false>(std::forward<std::remove_reference_t<T>>(obj))) {}

   private:
    template <bool is_function, bool is_objref, typename LAMBDA>
    ZC_CLOSURE_TYPE wrap_lambda(LAMBDA&& lambda) {
        constexpr bool is_lvalue_param = std::is_invocable_v<LAMBDA, ZCPP_PARAM&>;
        typedef std::conditional_t<is_function, std::function<std::remove_reference_t<LAMBDA>>,
                                   std::remove_reference_t<LAMBDA>>
            CONTEXT_TYPE;

        void* context;
        ZC_RETVAL (*call)(ZC_PARAM*, void*);
        void (*drop)(void*);

        if constexpr (is_function) {
            context = new CONTEXT_TYPE(lambda);
        } else if constexpr (is_objref) {
            context = &lambda;
        } else {
            context = new LAMBDA(std::move(lambda));
        }

        if constexpr (is_lvalue_param) {
            call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
                ZCPP_PARAM wrapper(pvalue);
                if constexpr (std::is_same_v<ZC_RETVAL, void>) {
                    static_cast<CONTEXT_TYPE*>(ctx)->operator()(wrapper);
                    *pvalue = wrapper.take();
                } else {
                    auto r = static_cast<CONTEXT_TYPE*>(ctx)->operator()(wrapper);
                    *pvalue = wrapper.take();
                    return r;
                }
            };
            drop = [](void* ctx) {
                ZCPP_PARAM wrapper(nullptr);
                static_cast<CONTEXT_TYPE*>(ctx)->operator()(wrapper);
                if constexpr (!is_objref) delete static_cast<CONTEXT_TYPE*>(ctx);
            };
        } else {
            call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
                return static_cast<CONTEXT_TYPE*>(ctx)->operator()(ZCPP_PARAM(pvalue));
            };
            drop = [](void* ctx) {
                static_cast<CONTEXT_TYPE*>(ctx)->operator()(ZCPP_PARAM(nullptr));
                if constexpr (!is_objref) delete static_cast<CONTEXT_TYPE*>(ctx);
            };
        }
        return {context, call, drop};
    };
};

}  // namespace zenohcxx
