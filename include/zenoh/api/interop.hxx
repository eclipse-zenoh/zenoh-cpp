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

#include <optional>
#include <type_traits>

#include "base.hxx"

namespace zenoh::interop {

/// @brief Get zenoh-c representation of trivially copyable zenoh-cpp object.
template <class CopyableType>
CopyableType* as_copyable_c_ptr(Copyable<CopyableType>& cpp_obj) {
    return reinterpret_cast<CopyableType*>(&cpp_obj);
}

/// @brief Get zenoh-c representation of trivially copyable zenoh-cpp object.
template <class CopyableType>
const CopyableType* as_copyable_c_ptr(const Copyable<CopyableType>& cpp_obj) {
    return reinterpret_cast<const CopyableType*>(&cpp_obj);
}

/// @brief Get zenoh-c representation of std::optional of trivially copyable zenoh-cpp object.
/// @return pointer to zenoh-c representation of trivially copyable zenoh-cpp object, or NULL if cpp_obj is empty.
template <class CopyableCppObj>
auto* as_copyable_c_ptr(std::optional<CopyableCppObj>& cpp_obj) {
    return cpp_obj.has_value() ? as_copyable_c_ptr(cpp_obj.value()) : nullptr;
}

/// @brief Get zenoh-c representation of std::optional of trivially copyable zenoh-cpp object.
/// @return pointer to zenoh-c representation of trivially copyable zenoh-cpp object, or NULL if cpp_obj is empty.
template <class CopyableCppObj>
const auto* as_copyable_c_ptr(const std::optional<CopyableCppObj>& cpp_obj) {
    return cpp_obj.has_value() ? as_copyable_c_ptr(cpp_obj.value()) : nullptr;
}

/// @brief Get owned zenoh-c representation of owned zenoh-cpp object.
template <class OwnedType>
OwnedType* as_owned_c_ptr(Owned<OwnedType>& cpp_obj) {
    return reinterpret_cast<OwnedType*>(&cpp_obj);
}

/// @brief Get owned zenoh-c representation of owned zenoh-cpp object.
template <class OwnedType>
const OwnedType* as_owned_c_ptr(const Owned<OwnedType>& cpp_obj) {
    return reinterpret_cast<const OwnedType*>(&cpp_obj);
}

/// @brief Get loaned zenoh-c representation of owned zenoh-cpp object.
template <
    class OwnedType,
    class Loaned = typename ::z_owned_to_loaned_type_t<OwnedType>::type,  // SFINAE here if no loaned type declared
    class LoanAvailable = std::enable_if_t<zenoh::detail::is_loan_available_v<OwnedType>, Loaned>  // SFINAE here if
                                                                                                   // immutable loan is
                                                                                                   // not available
    >
const Loaned* as_loaned_c_ptr(const Owned<OwnedType>& cpp_obj) {
    return ::z_loan(*as_owned_c_ptr(cpp_obj));
}

/// @brief Get loaned zenoh-c representation of owned zenoh-cpp object.
/// @return pointer to zenoh-c representation of loaned zenoh-cpp object, or NULL if cpp_obj is empty.
template <class OwnedCppObj>
const auto* as_loaned_c_ptr(const std::optional<OwnedCppObj>& cpp_obj) {
    return cpp_obj.has_value() ? as_loaned_c_ptr(*cpp_obj) : nullptr;
}

/// @brief Get loaned zenoh-c representation of owned zenoh-cpp object.
template <class OwnedType,
          class Loaned = typename ::z_owned_to_loaned_type_t<OwnedType>::type,  // SFINAE here if no loaned type
                                                                                // declared
          class LoanAvailable = std::enable_if_t<zenoh::detail::is_loan_mut_available_v<OwnedType>,
                                                 Loaned>  // SFINAE here if mutable loan is not available
          >
Loaned* as_loaned_c_ptr(Owned<OwnedType>& cpp_obj) {
    return ::z_loan_mut(*as_owned_c_ptr(cpp_obj));
}

/// @brief Get loaned zenoh-c representation of owned zenoh-cpp object.
/// @return pointer to zenoh-c representation of loaned zenoh-cpp object, or NULL if cpp_obj is empty.
template <class OwnedCppObj>
auto* as_loaned_c_ptr(std::optional<OwnedCppObj>& cpp_obj) {
    return cpp_obj.has_value() ? as_loaned_c_ptr(*cpp_obj) : nullptr;
}

/// @brief Get moved zenoh-c representation of owned zenoh-cpp object.
template <class OwnedType>
auto* as_moved_c_ptr(Owned<OwnedType>& cpp_obj) {
    return ::z_move(*as_owned_c_ptr(cpp_obj));
}

/// @brief Get moved zenoh-c representation of std::optional of owned zenoh-cpp object.
/// @return pointer to zenoh-c representation of moved owned zenoh-cpp object, or NULL if cpp_obj is empty.
template <class OwnedCppObj>
auto* as_moved_c_ptr(std::optional<OwnedCppObj>& cpp_obj) {
    return cpp_obj.has_value() ? ::z_move(*as_owned_c_ptr(cpp_obj.value())) : nullptr;
}

/// @brief Get zenoh-cpp representation of owned zenoh-c struct, from its loaned ptr.
template <class T, class LoanedType,
          class OwnedType = typename ::z_loaned_to_owned_type_t<LoanedType>::type  // SFINAE here if no loaned type
                                                                                   // declared
          >
const auto& as_owned_cpp_ref(const LoanedType* loaned_c_obj) {
    static_assert(sizeof(OwnedType) == sizeof(LoanedType) && alignof(OwnedType) == alignof(LoanedType),
                  "Loaned and owned classes must have the same layout");
    static_assert(sizeof(T) == sizeof(LoanedType) && alignof(T) == alignof(LoanedType),
                  "Loaned and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Owned<OwnedType>, T>, "Target class should be derived from an owned class");
    const OwnedType* o = reinterpret_cast<const OwnedType*>(loaned_c_obj);
    const zenoh::Owned<OwnedType>* o_cpp = reinterpret_cast<const zenoh::Owned<OwnedType>*>(o);
    return *reinterpret_cast<const T*>(o_cpp);
}

/// @brief Get zenoh-cpp representation of owned zenoh-c struct, from its loaned ptr.
template <class T, class LoanedType,
          class OwnedType = typename ::z_loaned_to_owned_type_t<LoanedType>::type  // SFINAE here if no loaned type
                                                                                   // declared
          >
auto& as_owned_cpp_ref(LoanedType* loaned_c_obj) {
    static_assert(sizeof(OwnedType) == sizeof(LoanedType) && alignof(OwnedType) == alignof(LoanedType),
                  "Loaned and owned classes must have the same layout");
    static_assert(sizeof(T) == sizeof(LoanedType) && alignof(T) == alignof(LoanedType),
                  "Loaned and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Owned<OwnedType>, T>, "Target class should be derived from an owned class");
    OwnedType* o = reinterpret_cast<OwnedType*>(loaned_c_obj);
    zenoh::Owned<OwnedType>* o_cpp = reinterpret_cast<zenoh::Owned<OwnedType>*>(o);
    return *reinterpret_cast<T*>(o_cpp);
}

/// @brief Get zenoh-cpp representation of owned zenoh-c struct.
template <class T, class OwnedType>
std::enable_if_t<std::is_base_of_v<zenoh::Owned<OwnedType>, T>, T&> as_owned_cpp_ref(OwnedType* owned_c_obj) {
    static_assert(sizeof(T) == sizeof(OwnedType) && alignof(T) == alignof(OwnedType),
                  "Owned and Target classes must have the same layout");
    zenoh::Owned<OwnedType>* o_cpp = reinterpret_cast<zenoh::Owned<OwnedType>*>(owned_c_obj);
    return *reinterpret_cast<T*>(o_cpp);
}

/// @brief Get zenoh-cpp representation of copyable zenoh-c struct.
template <class T, class CopyableType>
const auto& as_copyable_cpp_ref(const CopyableType* copyable_c_obj) {
    static_assert(sizeof(T) == sizeof(CopyableType) && alignof(T) == alignof(CopyableType),
                  "Copyable and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Copyable<CopyableType>, T>,
                  "Target class should be derived from a copyable class");
    const zenoh::Copyable<CopyableType>* c_cpp = reinterpret_cast<const zenoh::Copyable<CopyableType>*>(copyable_c_obj);
    return *reinterpret_cast<const T*>(c_cpp);
}

/// @brief Get zenoh-cpp representation of copyable zenoh-c struct.
template <class T, class CopyableType>
auto& as_copyable_cpp_ref(CopyableType* copyable_c_obj) {
    static_assert(sizeof(T) == sizeof(CopyableType) && alignof(T) == alignof(CopyableType),
                  "Copyable and Target classes must have the same layout");
    static_assert(std::is_base_of_v<zenoh::Copyable<CopyableType>, T>,
                  "Target class should be derived from a copyable class");
    zenoh::Copyable<CopyableType>* c_cpp = reinterpret_cast<zenoh::Copyable<CopyableType>*>(copyable_c_obj);
    return *reinterpret_cast<T*>(c_cpp);
}

/// @brief Move owned Zenoh zenoh-cpp object object into zenoh-c struct.
template <class OwnedType>
OwnedType move_to_c_obj(Owned<OwnedType>&& owned_cpp_obj) {
    OwnedType o = *as_owned_c_ptr(owned_cpp_obj);
    ::z_internal_null(as_owned_c_ptr(owned_cpp_obj));
    return o;
}

namespace detail {
template <class OwnedType>
bool check(const Owned<OwnedType>& owned_cpp_obj) {
    return ::z_internal_check(*as_owned_c_ptr(owned_cpp_obj));
}

struct Converter {
    template <class T>
    static T null_owned() {
        return T(zenoh::detail::null_object);
    }
    template <class T, class Inner = typename T::InnerType>
    static T copyable_to_cpp(const Inner& i) {
        return T(i);
    }

    template <class OPTIONS>
    static auto to_c_opts(OPTIONS& options) {
        return options.to_c_opts();
    }
};

template <class T>
T null() {
    return Converter::null_owned<T>();
}

}  // namespace detail

/// @brief Copy copyable zenoh-c struct into corresponding zenoh-cpp object.
template <class T, class CopyableType>
T into_copyable_cpp_obj(const CopyableType& copyable_c_obj) {
    return detail::Converter::copyable_to_cpp<T>(copyable_c_obj);
}

}  // namespace zenoh::interop
