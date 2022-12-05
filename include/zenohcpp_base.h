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

#include <optional>
#include <type_traits>
#include <utility>

#include "zenoh.h"

namespace zenoh {

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
    Owned() = delete;
    Owned(ZC_OWNED_TYPE* pv) {
        if (pv) {
            _0 = *pv;
            ::z_null(*pv);
        } else
            ::z_null(_0);
    }
    Owned& operator=(const Owned& v) = delete;
    Owned(const Owned& v) = delete;
    Owned(ZC_OWNED_TYPE&& v) : _0(v) { ::z_null(v); }
    Owned(Owned&& v) : Owned(std::move(v._0)) {}
    ~Owned() { ::z_drop(&_0); }
    ZC_OWNED_TYPE take() {
        auto r = _0;
        ::z_null(_0);
        return r;
    }
    void put(ZC_OWNED_TYPE& v) {
        ::z_drop(&_0);
        _0 = v;
        ::z_null(v);
    }
    explicit operator ZC_OWNED_TYPE&() { return _0; }
    bool check() const { return ::z_check(_0); }

   protected:
    ZC_OWNED_TYPE _0;
};

//
// Base type for C++ wrappers for Zenoh closures which doesn't take ownership of passed value
// It expects that
// - ZCPP_PARAM type is derived from ZC_PARAM
// - LAMBDA can be invoked as void(const ZCPP_PARAM*)
// - LAMBDA called with nullptr to notify that closure is being destructed
//
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<ZC_PARAM, ZCPP_PARAM>, bool> = true>
class ClosureConstPtrParam : public Owned<ZC_CLOSURE_TYPE> {
   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    void call(const ZC_PARAM* v) { Owned<ZC_CLOSURE_TYPE>::_0.call(v, this); }

    void operator()(const ZC_PARAM* v) { call(v); }

    void operator()(const ZCPP_PARAM* v) { call(v); }

    // Template constructors

    template <typename LAMBDA, typename std::enable_if_t<std::is_invocable_v<LAMBDA, const ZCPP_PARAM*>, bool> = true>
    ClosureConstPtrParam(LAMBDA&& lambda) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_to_closure(std::move(lambda))) {}

    template <typename FUNC, typename std::enable_if_t<std::is_invocable_v<FUNC, const ZCPP_PARAM*>, bool> = true>
    ClosureConstPtrParam(const FUNC& func) : Owned<ZC_CLOSURE_TYPE>(wrap_func_to_closure(func)) {}

    // TODO: more constructors to be added here

   private:
    template <typename LAMBDA>
    ZC_CLOSURE_TYPE wrap_lambda_to_closure(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](const ZC_PARAM* pvalue, void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(static_cast<const ZCPP_PARAM*>(pvalue));
            },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(nullptr);
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }

    template <typename FUNC>
    ZC_CLOSURE_TYPE wrap_func_to_closure(const FUNC& func) {
        auto lambda = [func](const ZC_PARAM* v) { func(static_cast<const ZCPP_PARAM*>(v)); };
        return wrap_lambda_to_closure(std::move(lambda));
    }
};

//
// Base type for C++ wrappers for Zenoh closures which takes ownership of passed value
// It expects that
// - ZCPP_PARAM is derived from Owned<ZC_PARAM>
//
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<Owned<ZC_PARAM>, ZCPP_PARAM>, bool> = true>

class ClosureMoveParam : public Owned<ZC_CLOSURE_TYPE> {
   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    void call(ZC_PARAM* v) { Owned<ZC_CLOSURE_TYPE>::_0.call(v, this); }

    void operator()(ZC_PARAM* v) { call(v); }

    void operator()(ZCPP_PARAM& v) { call(&(static_cast<ZC_PARAM&>(v))); }

    // Template constructors

    template <typename LAMBDA, typename std::enable_if_t<std::is_invocable_v<LAMBDA, ZCPP_PARAM> &&
                                                             !std::is_invocable_v<LAMBDA, std::optional<ZCPP_PARAM>>,
                                                         bool> = true>
    ClosureMoveParam(LAMBDA&& lambda) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_lvalue_param<LAMBDA>(std::move(lambda))) {}

    template <typename LAMBDA, typename std::enable_if_t<std::is_invocable_v<LAMBDA, ZCPP_PARAM&>, bool> = true>
    ClosureMoveParam(LAMBDA&& lambda) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_rvalue_param<LAMBDA>(std::move(lambda))) {}

    template <typename LAMBDA,
              typename std::enable_if_t<std::is_invocable_v<LAMBDA, std::optional<ZCPP_PARAM>>, bool> = true>
    ClosureMoveParam(LAMBDA&& lambda) : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_optional_param<LAMBDA>(std::move(lambda))) {}

    template <typename LAMBDA, typename std::enable_if_t<std::is_invocable_v<LAMBDA, ZCPP_PARAM> &&
                                                             !std::is_invocable_v<LAMBDA, std::optional<ZCPP_PARAM>>,
                                                         bool> = true>
    ClosureMoveParam(const LAMBDA& func)
        : ClosureMoveParam(std::move([func](ZCPP_PARAM&& v) { func(std::forward<ZCPP_PARAM>(v)); })) {}

    // template <typename FUNC, typename PARAM>
    // ClosureMoveParam(const FUNC& func) : ClosureMoveParam(std::move([func](PARAM v) { func(v); })) {}

    // template <typename FUNC>
    // Closure(const FUNC& func) : Owned<ZC_CLOSURE_TYPE>(wrap_func_to_closure<FUNC>(func)) {}

    // TODO: more constructors to be added here

   private:
    template <typename LAMBDA>
    ZC_CLOSURE_TYPE wrap_lambda_lvalue_param(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](ZC_PARAM* pvalue, void* ctx) { static_cast<LAMBDA*>(ctx)->operator()(ZCPP_PARAM(pvalue)); },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(ZCPP_PARAM(nullptr));
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }

    template <typename LAMBDA>
    ZC_CLOSURE_TYPE wrap_lambda_rvalue_param(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](ZC_PARAM* pvalue, void* ctx) {
                ZCPP_PARAM wrapper(pvalue);
                static_cast<LAMBDA*>(ctx)->operator()(wrapper);
                *pvalue = wrapper.take();
            },
            drop : [](void* ctx) {
                ZCPP_PARAM empty(nullptr);
                static_cast<LAMBDA*>(ctx)->operator()(empty);
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }

    template <typename LAMBDA>
    ZC_CLOSURE_TYPE wrap_lambda_optional_param(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](ZC_PARAM* pvalue, void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_PARAM>(std::in_place, pvalue));
            },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_PARAM>());
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }

    // template <typename FUNC, typename LAMBDA>
    // LAMBDA wrap_func_to_closure(const FUNC& func) {
    //     auto lambda = [func](std::optional<ZCPP_PARAM> v) { func(v); };
    //     return wrap_lambda_to_closure(std::move(lambda));
    // }
};

}  // namespace zenoh