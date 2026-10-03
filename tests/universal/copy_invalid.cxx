//
// Copyright (c) 2025 ZettaScale Technology
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

// Copying or cloning an invalid (null, moved-from or failed to construct) object
// must not loan it, and must produce an invalid object.

#include "zenoh.hxx"
#undef NDEBUG
#include <assert.h>

using namespace zenoh;

template <class T>
void check_copy_invalid(const T& invalid, const T& valid) {
    assert(!interop::detail::check(invalid));
    T copy(invalid);
    assert(!interop::detail::check(copy));
    T assigned(valid);
    assert(interop::detail::check(assigned));
    assigned = invalid;
    assert(!interop::detail::check(assigned));
}

template <class T>
void check_clone_invalid() {
    T invalid = interop::detail::null<T>();
    T copy = invalid.clone();
    assert(!interop::detail::check(copy));
}

#ifdef ZENOHCXX_ZENOHC  // Pico does not validate key expressions yet.
void keyexpr_failed_construction() {
    ZResult err = Z_OK;
    KeyExpr invalid("a/**/**/c", false, &err);
    assert(err != Z_OK);
    check_copy_invalid(invalid, KeyExpr("a/b"));
}
#endif

void keyexpr_moved_from() {
    KeyExpr k("a/b");
    KeyExpr moved(std::move(k));
    check_copy_invalid(k, KeyExpr("a/b"));
}

void bytes_moved_from() {
    Bytes b("data");
    Bytes moved(std::move(b));
    assert(!interop::detail::check(b));
    Bytes copy = b.clone();
    assert(!interop::detail::check(copy));
}

int main(int argc, char** argv) {
#ifdef ZENOHCXX_ZENOHC
    keyexpr_failed_construction();
#endif
    keyexpr_moved_from();
    bytes_moved_from();
    check_clone_invalid<Sample>();
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_QUERY == 1
    check_clone_invalid<Reply>();
#endif
    check_clone_invalid<Query>();
}
