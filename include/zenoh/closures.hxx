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
// This file contains structures and classes API without implementations
//

#pragma once

#include "base.hxx"

namespace zenoh::detail::closures {

struct IDroppable {
    virtual void drop() {};
    virtual ~IDroppable() {
        drop();
    }

    static void delete_from_context(void* context) {
        delete reinterpret_cast<IDroppable*>(context);
    }
};

template<class R, class ...Args>
struct IClosure : public IDroppable {
    virtual R call(Args... args) = 0;

    static R call_from_context(void* context, Args... args) {
        IDroppable* d = reinterpret_cast<IDroppable*>(context);
        return dynamic_cast<IClosure<R, Args...>*>(d)->call(args...);
    }
};


template<class C, class D, class R, class ...Args>
class Closure : public IClosure<R, Args...> {
    typename std::conditional_t<std::is_lvalue_reference_v<C>, C, std::remove_reference_t<C>> _call;
    typename std::conditional_t<std::is_lvalue_reference_v<D>, D, std::remove_reference_t<D>> _drop;
public:
    Closure(C&& call, D&& drop)
        : _call(std::forward<C>(call)), _drop(std::forward<D>(drop))
    {}

    virtual R call(Args... args) override {
        return _call(std::forward<Args>(args)...);
    }

    virtual void drop() override {
        return _drop();
    }

    static void* into_context(C&& call, D&& drop) {
        auto obj = new Closure<C, D, R, Args...>(std::forward<C>(call), std::forward<D>(drop));
        auto d = dynamic_cast<IDroppable*>(obj);
        return reinterpret_cast<void*>(d);
    }
};



/// Ensure that function pointers are defined with extern C linkage
extern "C" {
    inline void _zenoh_on_drop(void* context) {
        IDroppable::delete_from_context(context);
    }
}

}

namespace zenoh::closures {
    inline void none() {};
    using None = decltype(&none);
}