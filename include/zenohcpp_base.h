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
    bool check() const { return ::z_check(_0); }

   protected:
    ZC_OWNED_TYPE _0;
};

//
// Base type for C++ wrappers for Zenoh closures which doesn't take ownership of passed value
// It expects that
// - ZCPP_CALL_PARAM type is derived from ZC_CALL_PARAM
// - LAMBDA can be invoked as void(const ZCPP_CALL_PARAM*)
// - LAMBDA called with nullptr to notify that closure is being destructed
//
template <typename ZC_CLOSURE_TYPE, typename ZC_CALL_PARAM, typename ZCPP_CALL_PARAM,
          typename std::enable_if<std::is_base_of<ZC_CALL_PARAM, ZCPP_CALL_PARAM>, bool> = true>
    > class ClosureConstPtrParam : public Owned<ZC_CLOSURE_TYPE> {
   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    void call(const ZC_CALL_PARAM* v) { Owned<ZC_CLOSURE_TYPE>::_0.call(v, this); }

    void operator()(const ZC_CALL_PARAM* v) { call(v); }

    void operator()(const ZCPP_CALL_PARAM* v) { call(v); }

    // Template constructors

    template <typename LAMBDA>
    Closure(LAMBDA&& lambda)
        : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_to_closure<LAMBDA, ZCPP_CALL_PARAM>(std::move(lambda))) {}

    // template <typename FUNC, typename ZCPP_CALL_PARAM = ZCPP_CALL_PARAM_DEFAULT>
    // Closure(const FUNC& func) : Owned<ZC_CLOSURE_TYPE>(wrap_func_to_closure<FUNC, ZCPP_CALL_PARAM>(func)) {}

    // TODO: more constructors to be added here

   private:
    // template <typename FUNC, typename ZCPP_CALL_PARAM>
    // ZC_CLOSURE_TYPE wrap_func_to_closure(const FUNC& func) {
    //     auto lambda = [func](std::optional<ZCPP_CALL_PARAM> v) { func(v); };
    //     return wrap_lambda_to_closure(std::move(lambda));
    // }

    template <typename LAMBDA,
              typename std::enable_if<std::is_invocable_v<LAMBDA, const ZCPP_CALL_PARAM*>, bool> = true>
    ZC_CLOSURE_TYPE wrap_lambda_to_closure(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](const ZC_CALL_PARAM* pvalue, void* ctx) { static_cast<LAMBDA*>(ctx)->operator()(pvalue); },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(nullptr);
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }
};

//
// Base type for C++ wrappers for Zenoh closures which takes ownership of passed value
// It expects that
// - ZCPP_CALL_PARAM is derived from Owned<ZC_CALL_PARAM>
// - LAMBDA can be invoked as void(std::optional<ZCPP_CALL_PARAM>)
// - LABDA is called with empty std::optional when closure is being destructed
//
template <typename ZC_CLOSURE_TYPE, typename ZC_CALL_PARAM, typename ZCPP_CALL_PARAM,
          typename std::enable_if<
              std::is_base_of<Owned<std::remove_pointer_t<ZC_CALL_PARAM>>, std::remove_reference_t<ZCPP_CALL_PARAM>>,
              bool> = true>

    > class ClosureMoveParam : public Owned<ZC_CLOSURE_TYPE> {
   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    void call(ZC_CALL_PARAM v) { Owned<ZC_CLOSURE_TYPE>::_0.call(v, this); }

    void operator()(ZC_CALL_PARAM* v) { call(v); }

    void operator()(ZCPP_CALL_PARAM& v) {
        auto zv = v.take();
        call(&zv);
        v.put(zv);
    }

        // Template constructors

    template <typename LAMBDA>
    Closure(LAMBDA&& lambda)
        : Owned<ZC_CLOSURE_TYPE>(wrap_lambda_to_closure<LAMBDA, ZCPP_CALL_PARAM>(std::move(lambda))) {}

    // template <typename FUNC, typename ZCPP_CALL_PARAM = ZCPP_CALL_PARAM_DEFAULT>
    // Closure(const FUNC& func) : Owned<ZC_CLOSURE_TYPE>(wrap_func_to_closure<FUNC, ZCPP_CALL_PARAM>(func)) {}

    // TODO: more constructors to be added here

   private:
    // template <typename FUNC, typename ZCPP_CALL_PARAM>
    // ZC_CLOSURE_TYPE wrap_func_to_closure(const FUNC& func) {
    //     auto lambda = [func](std::optional<ZCPP_CALL_PARAM> v) { func(v); };
    //     return wrap_lambda_to_closure(std::move(lambda));
    // }

    template <typename LAMBDA, typename std::enable_if<std::is_invocable_v<LAMBDA, ZCPP_CALL_PARAM>, bool> = true>
    ZC_CLOSURE_TYPE wrap_lambda_to_closure(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](ZC_CALL_PARAM pvalue, void* ctx) { static_cast<LAMBDA*>(ctx)->operator()(pvalue); },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_CALL_PARAM>());
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }

    template <typename LAMBDA, typename ZCPP_CALL_PARAM>
    ZC_CLOSURE_TYPE wrap_lambda_to_closure(LAMBDA&& lambda) {
        return {
            context : new LAMBDA(std::move(lambda)),
            call : [](ZC_CALL_PARAM pvalue, void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(
                    std::optional<ZCPP_CALL_PARAM>(std::in_place, std::move(*pvalue)));
            },
            drop : [](void* ctx) {
                static_cast<LAMBDA*>(ctx)->operator()(std::optional<ZCPP_CALL_PARAM>());
                delete static_cast<LAMBDA*>(ctx);
            },
        };
    }
};

}  // namespace zenoh