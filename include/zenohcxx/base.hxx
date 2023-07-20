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
class ClosureConstRefParam : public Owned<ZC_CLOSURE_TYPE> {
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    // Call closure with pointer to C parameter
    ZC_RETVAL call(ZC_PARAM* v) {
        if (check()) Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context);
    }

    // Call closure with const reference to C++ parameter
    ZC_RETVAL operator()(const ZCPP_PARAM& v) { return call(&(static_cast<const ZC_PARAM&>(v))); }

    // Construct empty closure
    ClosureConstRefParam() : Owned<ZC_CLOSURE_TYPE>(nullptr) {}

    // Construct closure from the data handler: any object with operator()(const ZCPP_PARAM&) defined
    template <typename T>
    ClosureConstRefParam(T&& obj) : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(obj))) {}

    // Construct closure from the data handler and the drop handler
    // data handler is any object with operator()(const ZCPP_PARAM&) defined
    // drop handler is any object with operator()() defined
    //
    // Drop handler is convenient when it's necessary to catch dropping of the closure costructed from function pointer,
    // object lvalue reference or lambda. If the closure holds the user's object, the additional drop handler is
    // probably excessive. The cleanup in this case may be done in the object's destructor.
    template <typename T, typename D>
    ClosureConstRefParam(T&& on_call, D&& on_drop)
        : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(on_call), std::forward<D>(on_drop))) {}

   private:
    typedef ZC_RETVAL (*CALL)(const ZCPP_PARAM& pvalue);
    struct Call {
        CALL func;
    };
    typedef void (*DROP)();
    struct Drop {
        DROP func;
    };
    ZC_CLOSURE_TYPE wrap_call(CALL on_call) {
        auto context = new Call{on_call};
        auto call = [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto on_call = static_cast<Call*>(ctx);
            return on_call->func(*static_cast<const ZCPP_PARAM*>(pvalue));
        };
        auto drop = [](void* ctx) { delete static_cast<Call*>(ctx); };
        return {context, call, drop};
    }
    template <typename T>
    ZC_CLOSURE_TYPE wrap_call(T& on_call) {
        auto context = &on_call;
        auto call = [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto on_call = static_cast<T*>(ctx);
            return (*on_call)(*static_cast<const ZCPP_PARAM*>(pvalue));
        };
        return {context, call, nullptr};
    }
    template <typename T>
    ZC_CLOSURE_TYPE wrap_call(T&& on_call) {
        auto context = new T(std::move(on_call));
        auto call = [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto on_call = static_cast<T*>(ctx);
            return (*on_call)(*static_cast<const ZCPP_PARAM*>(pvalue));
        };
        auto drop = [](void* ctx) {
            auto on_call = static_cast<T*>(ctx);
            delete on_call;
        };
        return {context, call, drop};
    }

    ZC_CLOSURE_TYPE wrap_call(CALL on_call, DROP on_drop) {
        auto context = new std::pair{Call{on_call}, Drop{on_drop}};
        auto call = [](const ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto pair = static_cast<std::pair<Call, Drop>*>(ctx);
            return pair->first.func(*static_cast<const ZCPP_PARAM*>(pvalue));
        };
        auto drop = [](void* ctx) {
            auto pair = static_cast<std::pair<Call, Drop>*>(ctx);
            pair->second.func();
            delete pair;
        };
        return {context, call, drop};
    }
};

//
// Base type for C++ wrappers for Zenoh closures which takes ownership of passed value
// It expects that
// - ZCPP_PARAM is derived from Owned<ZC_PARAM>
// - user's LAMBDA can be invoked with ZCPP_PARAM&&
//
template <typename ZC_CLOSURE_TYPE, typename ZC_PARAM, typename ZCPP_PARAM,
          typename std::enable_if_t<std::is_base_of_v<Owned<ZC_PARAM>, ZCPP_PARAM>, bool> = true>

class ClosureMoveParam : public Owned<ZC_CLOSURE_TYPE> {
    // The `z_owned_reply_channel_closure_t::call` have the return type `bool` instead of void
    // So have to use `decltype` to get the return type of the closure
    typedef decltype((*ZC_CLOSURE_TYPE::call)(nullptr, nullptr)) ZC_RETVAL;

   public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;

    // Closure is valid if it can be called. The drop operation is optional
    bool check() const { return Owned<ZC_CLOSURE_TYPE>::_0.call != nullptr; }

    // Call closure with pointer to C parameter
    ZC_RETVAL call(ZC_PARAM* v) {
        if constexpr (std::is_same_v<ZC_RETVAL, void>) {
            if (check()) Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context);
        } else {
            return check() ? Owned<ZC_CLOSURE_TYPE>::_0.call(v, Owned<ZC_CLOSURE_TYPE>::_0.context) : ZC_RETVAL{};
        }
    }

    // Call closure with reference to C++ parameter
    ZC_RETVAL operator()(ZCPP_PARAM&& v) { return call(&(static_cast<ZC_PARAM&>(v))); }

    // Construct closure from the data handler: any object with operator()(ZCPP_PARAM&&) defined
    template <typename T>
    ClosureMoveParam(T&& on_call) : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(on_call), []() {})) {}

    // Construct closure from the data handler and drop handler:
    // data handler: any object with operator()(ZCPP_PARAM&&) defined
    // drop handler: any object with operator()() defined
    //
    // Drop handler is convenient when it's necessary to catch dropping of the closure costructed from function pointer,
    // object lvalue reference or lambda. If the closure holds the user's object, the additional drop handler is
    // probably excessive. The cleanup in this case may be done in the object's destructor.
    template <typename T, typename D>
    ClosureMoveParam(T&& on_call, D&& on_drop)
        : Owned<ZC_CLOSURE_TYPE>(wrap_call(std::forward<T>(on_call), std::forward<D>(on_drop))) {}

   private:
    typedef ZC_RETVAL (*CALL)(ZCPP_PARAM&& pvalue);
    typedef void (*DROP)();

    template <typename C, typename D>
    ZC_CLOSURE_TYPE wrap_call(C&& on_call, D&& on_drop) {
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_call)>, C&, C> SC;
        typedef std::conditional_t<std::is_lvalue_reference_v<decltype(on_drop)>, D&, D> SD;
        auto context = new std::pair<SC, SD>{std::forward<C>(on_call), std::forward<D>(on_drop)};
        auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            return pair->first(ZCPP_PARAM(pvalue));
        };
        auto drop = [](void* ctx) {
            auto pair = static_cast<std::pair<SC, SD>*>(ctx);
            pair->second();
            delete pair;
        };
        return {context, call, drop};
    }

    // ZC_CLOSURE_TYPE wrap_call(CALL on_call, DROP on_drop) {
    //     auto context = new std::pair{on_call, on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<CALL, DROP>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<CALL, DROP>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T>
    // ZC_CLOSURE_TYPE wrap_call(T& on_call, DROP on_drop) {
    //     auto context = new std::pair<T&, DROP>{on_call, on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T&, DROP>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T, DROP>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T>
    // ZC_CLOSURE_TYPE wrap_call(T&& on_call, DROP on_drop) {
    //     auto context = new std::pair<T, DROP>{std::move(on_call), on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T, DROP>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T, DROP>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename D>
    // ZC_CLOSURE_TYPE wrap_call(CALL on_call, D& on_drop) {
    //     auto context = new std::pair{on_call, &on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<CALL, D*>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<CALL, D*>*>(ctx);
    //         (*pair->second)();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T, typename D>
    // ZC_CLOSURE_TYPE wrap_call(T& on_call, D& on_drop) {
    //     auto context = new std::pair{&on_call, &on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T*, D*>*>(ctx);
    //         return (*pair->first)(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T*, D*>*>(ctx);
    //         (*pair->second)();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T, typename D>
    // ZC_CLOSURE_TYPE wrap_call(T&& on_call, D& on_drop) {
    //     auto context = new std::pair<T, D*>{std::move(on_call), &on_drop};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T, D*>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T, D*>*>(ctx);
    //         (*pair->second)();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename D>
    // ZC_CLOSURE_TYPE wrap_call(CALL on_call, D&& on_drop) {
    //     auto context = new std::pair{on_call, std::move(on_drop)};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<CALL, D>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<CALL, D>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T, typename D>
    // ZC_CLOSURE_TYPE wrap_call(T& on_call, D&& on_drop) {
    //     auto context = new std::pair{&on_call, std::move(on_drop)};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T*, D>*>(ctx);
    //         return (*pair->first)(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T*, D>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }

    // template <typename T, typename D>
    // ZC_CLOSURE_TYPE wrap_call(T&& on_call, D&& on_drop) {
    //     auto context = new std::pair<T, D>{std::move(on_call), std::move(on_drop)};
    //     auto call = [](ZC_PARAM* pvalue, void* ctx) -> ZC_RETVAL {
    //         auto pair = static_cast<std::pair<T, D>*>(ctx);
    //         return pair->first(ZCPP_PARAM(pvalue));
    //     };
    //     auto drop = [](void* ctx) {
    //         auto pair = static_cast<std::pair<T, D>*>(ctx);
    //         pair->second();
    //         delete pair;
    //     };
    //     return {context, call, drop};
    // }
};

}  // namespace zenohcxx
