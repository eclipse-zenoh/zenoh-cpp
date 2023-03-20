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

// Do not add '#pragma once' and '#include` statements here
// as this file is included multiple times into different namespaces

inline QueryTarget query_target_default() { return ::z_query_target_default(); }

inline const char* as_cstr(z::WhatAmI whatami) {
    return whatami == z::WhatAmI::Z_WHATAMI_ROUTER   ? "Router"
           : whatami == z::WhatAmI::Z_WHATAMI_PEER   ? "Peer"
           : whatami == z::WhatAmI::Z_WHATAMI_CLIENT ? "Client"
                                                     : nullptr;
}

inline void init_logger() {
#ifdef __ZENOHCXX_ZENOHC
    ::zc_init_logger();
#endif
}

inline ::z_bytes_t BytesView::init(const uint8_t* start, size_t len) {
    ::z_bytes_t ret = {.start = start,
                       .len = len
#ifdef __ZENOHCXX_ZENOHPICO
                       ,
                       ._is_alloc = false
#endif
    };
    return ret;
}

inline std::ostream& operator<<(std::ostream& os, const z::Id& id) {
    for (size_t i = 0; id.id[i] != 0 && i < 16; i++)
        os << std::hex << std::setfill('0') << std::setw(2) << (int)id.id[i];
    return os;
}

inline const Id& HelloView::get_id() const {
#ifdef __ZENOHCXX_ZENOHC
    return static_cast<const z::Id&>(pid);
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    assert(zid.len == sizeof(Id));  // TODO: is this invariant that Id is always 16 bytes?
    return reinterpret_cast<const z::Id&>(*zid.start);
#endif
}

inline bool _split_ret_to_bool_and_err(int8_t ret, ErrNo& error) {
    if (ret < 0) {
        error = ret;
        return false;
    } else {
        error = 0;
        return ret == 0;
    }
}

inline bool KeyExprView::equals(const KeyExprView& v, ErrNo& error) const {
    return _split_ret_to_bool_and_err(::z_keyexpr_equals(*this, v), error);
}
inline bool KeyExprView::equals(const KeyExprView& v) const {
    ErrNo error;
    return equals(v, error);
}
inline bool KeyExprView::includes(const KeyExprView& v, ErrNo& error) const {
    return _split_ret_to_bool_and_err(::z_keyexpr_includes(*this, v), error);
}
inline bool KeyExprView::includes(const KeyExprView& v) const {
    ErrNo error;
    return includes(v, error);
}
inline bool KeyExprView::intersects(const KeyExprView& v, ErrNo& error) const {
    return _split_ret_to_bool_and_err(::z_keyexpr_intersects(*this, v), error);
}
inline bool KeyExprView::intersects(const KeyExprView& v) const {
    ErrNo error;
    return includes(v, error);
}

inline bool keyexpr_canonize(std::string& s, ErrNo& error) {
    uintptr_t len = s.length();
    error = ::z_keyexpr_canonize(&s[0], &len);
    s.resize(len);
    return error == 0;
}

inline bool keyexpr_canonize(std::string& s) {
    ErrNo error;
    return z::keyexpr_canonize(s, error);
}

inline bool keyexpr_is_canon(const std::string_view& s, ErrNo& error) {
    error = ::z_keyexpr_is_canon(s.begin(), s.length());
    return error == 0;
}

inline bool keyexpr_is_canon(const std::string_view& s) {
    ErrNo error;
    return z::keyexpr_is_canon(s, error);
}

inline bool Query::reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options,
                         ErrNo& error) const {
    return reply_impl(key, payload, &options, error);
}
inline bool Query::reply(KeyExprView key, const BytesView& payload, const QueryReplyOptions& options) const {
    ErrNo error;
    return reply_impl(key, payload, &options, error);
}
inline bool Query::reply(KeyExprView key, const BytesView& payload, ErrNo& error) const {
    return reply_impl(key, payload, nullptr, error);
}
inline bool Query::reply(KeyExprView key, const BytesView& payload) const {
    ErrNo error;
    return reply_impl(key, payload, nullptr, error);
}
inline bool Query::reply_impl(KeyExprView key, const BytesView& payload, const QueryReplyOptions* options,
                              ErrNo& error) const {
    error = ::z_query_reply(this, key, payload.start, payload.len, options);
    return error == 0;
}