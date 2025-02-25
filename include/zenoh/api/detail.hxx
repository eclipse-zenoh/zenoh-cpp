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

#include "../zenohc.hxx"

// namespace zenoh::detail
namespace zenoh::detail {
template <typename T, typename = void>
struct is_loan_available : std::false_type {};

template <typename T>
struct is_loan_available<T, std::void_t<decltype(::z_loan(std::declval<const T&>()))>> : std::true_type {};

template <typename T>
inline constexpr bool is_loan_available_v = is_loan_available<T>::value;

template <typename T, typename = void>
struct is_loan_mut_available : std::false_type {};

template <typename T>
struct is_loan_mut_available<T, std::void_t<decltype(::z_loan_mut(std::declval<T&>()))>> : std::true_type {};

template <typename T>
inline constexpr bool is_loan_mut_available_v = is_loan_mut_available<T>::value;

template <typename T, typename = void>
struct is_take_from_loaned_available : std::false_type {};

template <typename T>
struct is_take_from_loaned_available<
    T, std::void_t<decltype(::z_take_from_loaned(
           std::declval<T*>(), std::declval<typename ::z_owned_to_loaned_type_t<T>::type*>()))>> : std::true_type {};

template <typename T>
inline constexpr bool is_take_from_loaned_available_v = is_take_from_loaned_available<T>::value;

struct null_object_t {};
inline constexpr null_object_t null_object{};

}  // namespace zenoh::detail
