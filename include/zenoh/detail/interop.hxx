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

#include "../api/base.hxx"
#include <optional>

namespace zenoh::detail {

template<class CopyableType>
CopyableType* as_copyable_c_ptr(Copyable<CopyableType>& c) {
    return reinterpret_cast<CopyableType*>(&c);
}

template<class CopyableType>
const CopyableType* as_copyable_c_ptr(const Copyable<CopyableType>& c) {
    return reinterpret_cast<const CopyableType*>(&c);
}

template<class CopyableCppObj>
auto* as_copyable_c_ptr(std::optional<CopyableCppObj>& c) {
    return c.has_value() ? as_copyable_c_ptr(c.value()) : nullptr;
}

template<class CopyableCppObj>
const auto* as_copyable_c_ptr(const std::optional<CopyableCppObj>& c) {
    return c.has_value() ? as_copyable_c_ptr(c.value()) : nullptr;
}

template<class OwnedType>
OwnedType* as_owned_c_ptr(Owned<OwnedType>& o) {
    return reinterpret_cast<OwnedType*>(&o);
}

template<class OwnedType>
const OwnedType* as_owned_c_ptr(const Owned<OwnedType>& o) {
    return reinterpret_cast<const OwnedType*>(&o);
}

template<class OwnedCppObj>
auto* as_owned_c_ptr(std::optional<OwnedCppObj>& o) {
    return  o.has_value() ? as_owned_c_ptr(o.value()) : nullptr;
}

template<class OwnedCppObj>
const auto* as_owned_c_ptr(const std::optional<OwnedCppObj>& o) {
    return  o.has_value() ? as_owned_c_ptr(o.value()) : nullptr;
}

template<class OwnedType>
auto loan(const OwnedType& o) { 
    return ::z_loan(*as_owned_c_ptr(o)); 
}

template<class OwnedType>
auto loan(OwnedType& o) { 
    return ::z_loan_mut(*as_owned_c_ptr(o)); 
}

template<class T, class OwnedType>
auto& to_owned_cpp_ref(const OwnedType* o) {
    static_assert(sizeof(OwnedType) == sizeof(T) &&  alignof(OwnedType) == alignof(T),
        "Target and owned classes must have the same layout");
    return *reinterpret_cast<T*>(o);
}

template<class T, class LoanedType>
const auto& as_owned_cpp_obj(const LoanedType* l) {
    typedef typename z_loaned_to_owned_type_t<LoanedType>::type OwnedType;
    static_assert(sizeof(OwnedType) == sizeof(LoanedType) &&  alignof(OwnedType) == alignof(LoanedType),
        "Loaned and owned classes must have the same layout");
    static_assert(sizeof(T) == sizeof(LoanedType) &&  alignof(T) == alignof(LoanedType),
        "Loaned and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Owned<OwnedType>, T>,
        "Target class should be derived from an owned class");
    const OwnedType* o = reinterpret_cast<const OwnedType*>(l);
    const zenoh::Owned<OwnedType>* o_cpp = reinterpret_cast<const zenoh::Owned<OwnedType>*>(o);
    return *reinterpret_cast<const T*>(o_cpp);
}


template<class T, class CopyableType>
const auto& as_copyable_cpp_obj(const CopyableType* c) {
    static_assert(sizeof(T) == sizeof(CopyableType) &&  alignof(T) == alignof(CopyableType),
        "Copyable and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Copyable<CopyableType>, T>,
        "Target class should be derived from a copyable class");
    const zenoh::Copyable<CopyableType>* c_cpp = reinterpret_cast<const zenoh::Copyable<CopyableType>*>(c);
    return *reinterpret_cast<const T*>(c_cpp);
}

}