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

#ifdef __ZENOHCXX_ZENOHC
inline void init_logger() { ::zc_init_logger(); }
#endif

inline ::z_bytes_t z::BytesView::init(const uint8_t* start, size_t len) {
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

inline const z::Id& z::HelloView::get_id() const {
#ifdef __ZENOHCXX_ZENOHC
    return static_cast<const z::Id&>(pid);
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    assert(zid.len == sizeof(Id));  // TODO: is this invariant that Id is always 16 bytes?
    return reinterpret_cast<const z::Id&>(*zid.start);
#endif
}

inline bool _split_ret_to_bool_and_err(int8_t ret, ErrNo& error) {
    if (ret < -1) {
        error = ret;
        return false;
    } else {
        error = 0;
        return ret == 0;
    }
}

inline bool z::KeyExprView::equals(const z::KeyExprView& v, ErrNo& error) const {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_equals(*this, v), error);
}
inline bool z::KeyExprView::equals(const z::KeyExprView& v) const {
    ErrNo error;
    return equals(v, error);
}
inline bool z::KeyExprView::includes(const z::KeyExprView& v, ErrNo& error) const {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_includes(*this, v), error);
}
inline bool z::KeyExprView::includes(const z::KeyExprView& v) const {
    ErrNo error;
    return includes(v, error);
}
inline bool z::KeyExprView::intersects(const z::KeyExprView& v, ErrNo& error) const {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_intersects(*this, v), error);
}
inline bool z::KeyExprView::intersects(const z::KeyExprView& v) const {
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

inline bool z::Query::reply(z::KeyExprView key, const z::BytesView& payload, const QueryReplyOptions& options,
                            ErrNo& error) const {
    return reply_impl(key, payload, &options, error);
}
inline bool z::Query::reply(z::KeyExprView key, const z::BytesView& payload, const QueryReplyOptions& options) const {
    ErrNo error;
    return reply_impl(key, payload, &options, error);
}
inline bool z::Query::reply(z::KeyExprView key, const z::BytesView& payload, ErrNo& error) const {
    return reply_impl(key, payload, nullptr, error);
}
inline bool z::Query::reply(z::KeyExprView key, const z::BytesView& payload) const {
    ErrNo error;
    return reply_impl(key, payload, nullptr, error);
}
inline bool z::Query::reply_impl(z::KeyExprView key, const z::BytesView& payload, const QueryReplyOptions* options,
                                 ErrNo& error) const {
    error = ::z_query_reply(this, key, payload.start, payload.len, options);
    return error == 0;
}

#ifdef __ZENOHCXX_ZENOHC

inline std::variant<z::Config, ErrorMessage> config_from_file(const char* path) {
    z::Config config(::zc_config_from_file(path));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from file";
    }
}

inline std::variant<z::Config, ErrorMessage> config_from_str(const char* s) {
    z::Config config(::zc_config_from_str(s));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from string";
    }
}

inline std::variant<z::Config, ErrorMessage> config_client(const z::StrArrayView& peers) {
    z::Config config(::z_config_client(peers.val, peers.len));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from list of peers";
    }
}

inline std::variant<z::Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers) {
    std::vector<const char*> v(peers);
    return z::config_client(v);
}
#endif

inline z::ScoutingConfig z::Config::create_scouting_config() {
    return z::ScoutingConfig(::z_scouting_config_from(::z_loan(_0)));
}

inline bool z::Publisher::put(const z::BytesView& payload, const PublisherPutOptions& options, ErrNo& error) {
    return put_impl(payload, &options, error);
}
inline bool z::Publisher::put(const z::BytesView& payload, ErrNo& error) { return put_impl(payload, nullptr, error); }
inline bool z::Publisher::put(const z::BytesView& payload, const PublisherPutOptions& options) {
    ErrNo error;
    return put_impl(payload, &options, error);
}
inline bool z::Publisher::put(const z::BytesView& payload) {
    ErrNo error;
    return put_impl(payload, nullptr, error);
}

inline bool z::Publisher::delete_resource(const PublisherDeleteOptions& options, ErrNo& error) {
    return delete_impl(&options, error);
}
inline bool z::Publisher::delete_resource(ErrNo& error) { return delete_impl(nullptr, error); }
inline bool z::Publisher::delete_resource(const PublisherDeleteOptions& options) {
    ErrNo error;
    return delete_impl(&options, error);
}
inline bool z::Publisher::delete_resource() {
    ErrNo error;
    return delete_impl(nullptr, error);
}

inline bool z::Publisher::put_impl(const z::BytesView& payload, const PublisherPutOptions* options, ErrNo& error) {
    error = ::z_publisher_put(::z_loan(_0), payload.start, payload.len, options);
    return error == 0;
}

inline bool z::Publisher::delete_impl(const PublisherDeleteOptions* options, ErrNo& error) {
    error = ::z_publisher_delete(::z_loan(_0), options);
    return error == 0;
}

inline bool scout(z::ScoutingConfig&& config, ClosureHello&& callback, ErrNo& error) {
    auto c = config.take();
    auto cb = callback.take();
    error = ::z_scout(z_move(c), z_move(cb));
    return error == 0;
};

inline bool scout(z::ScoutingConfig&& config, ClosureHello&& callback) {
    ErrNo error;
    return z::scout(std::move(config), std::move(callback), error);
}

inline z::KeyExpr z::Session::declare_keyexpr(const z::KeyExprView& keyexpr) {
    return z::KeyExpr(::z_declare_keyexpr(::z_session_loan(&_0), keyexpr));
}

inline bool z::Session::undeclare_keyexpr(z::KeyExpr&& keyexpr, ErrNo& error) {
    return undeclare_keyexpr_impl(std::move(keyexpr), error);
}

inline bool z::Session::undeclare_keyexpr(z::KeyExpr&& keyexpr) {
    ErrNo error;
    return undeclare_keyexpr_impl(std::move(keyexpr), error);
}
inline bool z::Session::get(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback,
                            const GetOptions& options, ErrNo& error) {
    return get_impl(keyexpr, parameters, std::move(callback), &options, error);
}
inline bool z::Session::get(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback,
                            const GetOptions& options) {
    ErrNo error;
    return get_impl(keyexpr, parameters, std::move(callback), &options, error);
}
inline bool z::Session::get(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, ErrNo& error) {
    return get_impl(keyexpr, parameters, std::move(callback), nullptr, error);
}
inline bool z::Session::get(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback) {
    ErrNo error;
    return get_impl(keyexpr, parameters, std::move(callback), nullptr, error);
}

inline bool z::Session::put(z::KeyExprView keyexpr, const z::BytesView& payload, const PutOptions& options,
                            ErrNo& error) {
    return put_impl(keyexpr, payload, &options, error);
}
inline bool z::Session::put(z::KeyExprView keyexpr, const z::BytesView& payload, const PutOptions& options) {
    ErrNo error;
    return put_impl(keyexpr, payload, &options, error);
}
inline bool z::Session::put(z::KeyExprView keyexpr, const z::BytesView& payload, ErrNo& error) {
    return put_impl(keyexpr, payload, nullptr, error);
}
inline bool z::Session::put(z::KeyExprView keyexpr, const z::BytesView& payload) {
    ErrNo error;
    PutOptions options;
    return put_impl(keyexpr, payload, nullptr, error);
}

inline bool z::Session::delete_resource(z::KeyExprView keyexpr, const DeleteOptions& options, ErrNo& error) {
    return delete_impl(keyexpr, &options, error);
}
inline bool z::Session::delete_resource(z::KeyExprView keyexpr, const DeleteOptions& options) {
    ErrNo error;
    return delete_impl(keyexpr, &options, error);
}
inline bool z::Session::delete_resource(z::KeyExprView keyexpr, ErrNo& error) {
    return delete_impl(keyexpr, nullptr, error);
}
inline bool z::Session::delete_resource(z::KeyExprView keyexpr) {
    ErrNo error;
    PutOptions options;
    return delete_impl(keyexpr, nullptr, error);
}

inline std::variant<z::Queryable, ErrorMessage> z::Session::declare_queryable(z::KeyExprView keyexpr,
                                                                              ClosureQuery&& callback,
                                                                              const QueryableOptions& options) {
    return declare_queryable_impl(keyexpr, std::move(callback), &options);
}
inline std::variant<z::Queryable, ErrorMessage> z::Session::declare_queryable(z::KeyExprView keyexpr,
                                                                              ClosureQuery&& callback) {
    return declare_queryable_impl(keyexpr, std::move(callback), nullptr);
}

inline std::variant<z::Subscriber, ErrorMessage> z::Session::declare_subscriber(z::KeyExprView keyexpr,
                                                                                ClosureSample&& callback,
                                                                                const SubscriberOptions& options) {
    return declare_subscriber_impl(keyexpr, std::move(callback), &options);
}
inline std::variant<z::Subscriber, ErrorMessage> z::Session::declare_subscriber(z::KeyExprView keyexpr,
                                                                                ClosureSample&& callback) {
    return declare_subscriber_impl(keyexpr, std::move(callback), nullptr);
}

inline std::variant<z::PullSubscriber, ErrorMessage> z::Session::declare_pull_subscriber(
    z::KeyExprView keyexpr, ClosureSample&& callback, const PullSubscriberOptions& options) {
    return declare_pull_subscriber_impl(keyexpr, std::move(callback), &options);
}
inline std::variant<z::PullSubscriber, ErrorMessage> z::Session::declare_pull_subscriber(z::KeyExprView keyexpr,
                                                                                         ClosureSample&& callback) {
    return declare_pull_subscriber_impl(keyexpr, std::move(callback), nullptr);
}

inline std::variant<z::Publisher, ErrorMessage> z::Session::declare_publisher(z::KeyExprView keyexpr,
                                                                              const PublisherOptions& options) {
    return declare_publisher_impl(keyexpr, &options);
}
inline std::variant<z::Publisher, ErrorMessage> z::Session::declare_publisher(z::KeyExprView keyexpr) {
    return declare_publisher_impl(keyexpr, nullptr);
}

inline bool z::Session::info_routers_zid(ClosureZid&& callback, ErrNo& error) {
    auto c = callback.take();
    error = ::z_info_routers_zid(::z_session_loan(&_0), &c);
    return error == 0;
}
inline bool z::Session::info_routers_zid(ClosureZid&& callback) {
    auto c = callback.take();
    return ::z_info_routers_zid(::z_session_loan(&_0), &c) == 0;
}

inline bool z::Session::info_peers_zid(ClosureZid&& callback, ErrNo& error) {
    auto c = callback.take();
    error = ::z_info_peers_zid(::z_session_loan(&_0), &c);
    return error == 0;
}
inline bool z::Session::info_peers_zid(ClosureZid&& callback) {
    auto c = callback.take();
    return ::z_info_peers_zid(::z_session_loan(&_0), &c) == 0;
}

inline bool z::Session::undeclare_keyexpr_impl(KeyExpr&& keyexpr, ErrNo& error) {
    error = ::z_undeclare_keyexpr(::z_session_loan(&_0), &(static_cast<::z_owned_keyexpr_t&>(keyexpr)));
    return error == 0;
}

inline bool z::Session::get_impl(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback,
                                 const GetOptions* options, ErrNo& error) {
    auto c = callback.take();
    error = ::z_get(::z_session_loan(&_0), keyexpr, parameters, &c, options);
    return error == 0;
}

inline bool z::Session::put_impl(z::KeyExprView keyexpr, const z::BytesView& payload, const PutOptions* options,
                                 ErrNo& error) {
    error = ::z_put(::z_session_loan(&_0), keyexpr, payload.start, payload.len, options);
    return error == 0;
}

inline bool z::Session::delete_impl(z::KeyExprView keyexpr, const DeleteOptions* options, ErrNo& error) {
    error = ::z_delete(::z_session_loan(&_0), keyexpr, options);
    return error == 0;
}
inline std::variant<z::Queryable, ErrorMessage> z::Session::declare_queryable_impl(z::KeyExprView keyexpr,
                                                                                   ClosureQuery&& callback,
                                                                                   const QueryableOptions* options) {
    auto c = callback.take();
    z::Queryable queryable(::z_declare_queryable(::z_session_loan(&_0), keyexpr, &c, options));
    if (queryable.check()) {
        return std::move(queryable);
    } else {
        return "Unable to create queryable";
    }
}

inline std::variant<z::Subscriber, ErrorMessage> z::Session::declare_subscriber_impl(z::KeyExprView keyexpr,
                                                                                     ClosureSample&& callback,
                                                                                     const SubscriberOptions* options) {
    auto c = callback.take();
    z::Subscriber subscriber(::z_declare_subscriber(::z_session_loan(&_0), keyexpr, &c, options));
    if (subscriber.check()) {
        return std::move(subscriber);
    } else {
        return "Unable to create subscriber";
    }
}

inline std::variant<z::PullSubscriber, ErrorMessage> z::Session::declare_pull_subscriber_impl(
    z::KeyExprView keyexpr, ClosureSample&& callback, const PullSubscriberOptions* options) {
    auto c = callback.take();
    z::PullSubscriber pull_subscriber(::z_declare_pull_subscriber(::z_session_loan(&_0), keyexpr, &c, options));
    if (pull_subscriber.check()) {
        return std::move(pull_subscriber);
    } else {
        return "Unable to create pull subscriber";
    }
}

inline std::variant<z::Publisher, ErrorMessage> z::Session::declare_publisher_impl(z::KeyExprView keyexpr,
                                                                                   const PublisherOptions* options) {
    z::Publisher publisher(::z_declare_publisher(::z_session_loan(&_0), keyexpr, options));
    if (publisher.check()) {
        return std::move(publisher);
    } else {
        return "Unable to create publisher";
    }
}

inline ::z_owned_session_t z::Session::_z_open(z::Config&& v) {
    auto config = v.take();
    return ::z_open(z_move(config));
}

inline std::variant<z::Session, z::ErrorMessage> open(z::Config&& config, bool start_background_tasks) {
    z::Session session(std::move(config));
    if (!session.check()) {
        return "Unable to open session";
    }
#ifdef __ZENOHCXX_ZENOHPICO
    if (start_background_tasks) {
        if (!session.start_read_task()) {
            return "Unable to start read task";
        }
        if (!session.start_lease_task()) {
            return "Unable to start lease task";
        }
    }
#endif
    return std::move(session);
};

inline z::Session::~Session() {
#ifdef __ZENOHCXX_ZENOHPICO
    if (check()) {
        stop_read_task();
        stop_lease_task();
    }
#endif
}

#ifdef __ZENOHCXX_ZENOHPICO

inline bool z::Config::insert(uint8_t key, const char* value) {
    ErrNo error;
    return insert(key, std::move(value), error);
}

inline bool z::Config::insert(uint8_t key, const char* value, ErrNo& error) {
    error = ::zp_config_insert(::z_config_loan(&_0), key, ::z_string_make(value));
    return error == 0;
}

#endif

#ifdef __ZENOHCXX_ZENOHPICO
inline bool Session::start_read_task() {
    ErrNo error;
    return start_read_task(error);
}
inline bool Session::start_read_task(ErrNo& error) {
    error = ::zp_start_read_task(::z_session_loan(&_0), nullptr);
    return error == 0;
}
inline bool Session::stop_read_task() {
    ErrNo error;
    return stop_read_task(error);
}
inline bool Session::stop_read_task(ErrNo& error) {
    error = ::zp_stop_read_task(::z_session_loan(&_0));
    return error == 0;
}
inline bool Session::start_lease_task() {
    ErrNo error;
    return start_lease_task(error);
}
inline bool Session::start_lease_task(ErrNo& error) {
    error = ::zp_start_lease_task(::z_session_loan(&_0), nullptr);
    return error == 0;
}
inline bool Session::stop_lease_task() {
    ErrNo error;
    return stop_lease_task(error);
}
inline bool Session::stop_lease_task(ErrNo& error) {
    error = ::zp_stop_lease_task(::z_session_loan(&_0));
    return error == 0;
}
inline bool Session::read() {
    ErrNo error;
    return read(error);
}
inline bool Session::read(ErrNo& error) {
    error = ::zp_read(::z_session_loan(&_0), nullptr);
    return error == 0;
}
inline bool Session::send_keep_alive() {
    ErrNo error;
    return send_keep_alive(error);
}
inline bool Session::send_keep_alive(ErrNo& error) {
    error = ::zp_send_keep_alive(::z_session_loan(&_0), nullptr);
    return error == 0;
}
inline bool Session::send_join() {
    ErrNo error;
    return send_join(error);
}
inline bool Session::send_join(ErrNo& error) {
    error = ::zp_send_join(::z_session_loan(&_0), nullptr);
    return error == 0;
}
#endif
