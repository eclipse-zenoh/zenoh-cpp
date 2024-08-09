#pragma once

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

#include <iterator>

namespace zenoh::detail::commons {

template <typename It, typename F>
class TransformIterator {
    It _it;
    using Ftype = typename std::conditional_t<std::is_lvalue_reference_v<F>, F, std::remove_reference_t<F>>;
    Ftype _f;

   public:
    template <class FF>
    TransformIterator(It const& it, FF&& f) : _it(it), _f(std::forward<FF>(f)) {}

    using difference_type = typename std::iterator_traits<It>::difference_type;
    using value_type = typename std::invoke_result<Ftype, It>::type;
    using pointer = void;
    using reference = void;
    using iterator_category = std::input_iterator_tag;

    bool operator==(TransformIterator const& other) { return _it == other._it; }
    bool operator!=(TransformIterator const& other) { return _it != other._it; }

    auto operator*() const { return _f(_it); }

    auto operator++() {
        ++_it;
        return *this;
    }
    auto operator++(int) {
        auto prev = *this;
        ++_it;
        return prev;
    }
};

template <typename It, typename F>
auto make_transform_iterator(It const& it, F&& f) {
    return TransformIterator<It, F>(it, std::forward<F>(f));
}

}  // namespace zenoh::detail::commons