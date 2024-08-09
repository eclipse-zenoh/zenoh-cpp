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

#pragma once

#include <type_traits>
#include <utility>

namespace zenoh::detail::closures {

struct IDroppable {
    virtual void drop() = 0;
    virtual ~IDroppable(){};

    static void delete_from_context(void* context) {
        reinterpret_cast<IDroppable*>(context)->drop();
        delete reinterpret_cast<IDroppable*>(context);
    }

    void* as_context() {
        auto d = static_cast<IDroppable*>(this);
        return reinterpret_cast<void*>(d);
    }
};

template <class R, class... Args>
struct IClosure : public IDroppable {
    virtual R call(Args... args) = 0;

    static R call_from_context(void* context, Args... args) {
        IDroppable* d = reinterpret_cast<IDroppable*>(context);
        return static_cast<IClosure<R, Args...>*>(d)->call(args...);
    }
};

template <class D>
class Droppable : public IDroppable {
    typename std::conditional_t<std::is_lvalue_reference_v<D>, D, std::remove_reference_t<D>> _drop;

   public:
    template <class DD>
    Droppable(DD&& drop) : _drop(std::forward<DD>(drop)) {}

    virtual void drop() override { return _drop(); }

    template <class DD>
    static void* into_context(DD&& drop) {
        auto obj = new Droppable<D>(std::forward<DD>(drop));
        return obj->as_context();
    }
};

template <class C, class D, class R, class... Args>
class Closure : public IClosure<R, Args...> {
    typename std::conditional_t<std::is_lvalue_reference_v<C>, C, std::remove_reference_t<C>> _call;
    typename std::conditional_t<std::is_lvalue_reference_v<D>, D, std::remove_reference_t<D>> _drop;

   public:
    template <class CC, class DD>
    Closure(CC&& call, DD&& drop) : _call(std::forward<CC>(call)), _drop(std::forward<DD>(drop)) {}

    virtual R call(Args... args) override { return _call(std::forward<Args>(args)...); }

    virtual void drop() override { return _drop(); }

    template <class CC, class DD>
    static void* into_context(CC&& call, DD&& drop) {
        auto obj = new Closure<C, D, R, Args...>(std::forward<CC>(call), std::forward<DD>(drop));
        return obj->as_context();
    }
};

}  // namespace zenoh::detail::closures