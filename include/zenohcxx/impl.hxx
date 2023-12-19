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
    ::z_bytes_t ret = {len, start
#ifdef __ZENOHCXX_ZENOHPICO
                       ,
                       false
#endif
    };
    return ret;
}

inline std::ostream& operator<<(std::ostream& os, const z::Id& id) {
    for (size_t i = 0; id.id[i] != 0 && i < 16; i++)
        os << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(id.id[i]);
    return os;
}

inline const z::Id& z::HelloView::get_id() const {
#ifdef __ZENOHCXX_ZENOHC
    return static_cast<const z::Id&>(pid);
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    return static_cast<const z::Id&>(zid);
#endif
}

#ifdef __ZENOHCXX_ZENOHPICO
inline z::Str z::KeyExprView::resolve(const z::Session& s) const { return ::zp_keyexpr_resolve(s.loan(), *this); }
#endif

inline bool _split_ret_to_bool_and_err(int8_t ret, ErrNo& error) {
    if (ret < -1) {
        // return value less than -1 is an error code
        error = ret;
        return false;
    } else {
        // return value -1 is false, 0 is true
        error = 0;
        return ret == 0;
    }
}

inline bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_equals(a, b), error);
}
inline bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_includes(a, b), error);
}
inline bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::_split_ret_to_bool_and_err(::z_keyexpr_intersects(a, b), error);
}

#ifdef __ZENOHCXX_ZENOHC
inline bool keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::keyexpr_equals(a, b, error);
}
inline bool keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::keyexpr_includes(a, b, error);
}
inline bool keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::keyexpr_intersects(a, b, error);
}
#endif

#ifdef __ZENOHCXX_ZENOHC
inline z::KeyExpr keyexpr_concat(const z::KeyExprView& k, const std::string_view& s) {
    return ::z_keyexpr_concat(k, s.data(), s.length());
}
inline z::KeyExpr keyexpr_join(const z::KeyExprView& a, const z::KeyExprView& b) { return ::z_keyexpr_join(a, b); }
#endif

inline bool z::KeyExprView::equals(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_equals(*this, other, error);
}

inline bool z::KeyExprView::includes(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_includes(*this, other, error);
}

inline bool z::KeyExprView::intersects(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_intersects(*this, other, error);
}

inline bool z::KeyExpr::equals(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_equals(*this, other, error);
}

inline bool z::KeyExpr::includes(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_includes(*this, other, error);
}

inline bool z::KeyExpr::intersects(const z::KeyExprView& other, ErrNo& error) const {
    return z::keyexpr_intersects(*this, other, error);
}
#ifdef __ZENOHCXX_ZENOHC
inline z::KeyExpr z::KeyExprView::concat(const std::string_view& s) const { return z::keyexpr_concat(*this, s); }

inline z::KeyExpr z::KeyExprView::join(const z::KeyExprView& other) const { return z::keyexpr_join(*this, other); }

inline bool z::KeyExprView::equals(const z::KeyExprView& other) const { return z::keyexpr_equals(*this, other); }

inline bool z::KeyExprView::includes(const z::KeyExprView& other) const { return z::keyexpr_includes(*this, other); }
inline bool z::KeyExprView::intersects(const z::KeyExprView& other) const {
    return z::keyexpr_intersects(*this, other);
}

inline z::KeyExpr z::KeyExpr::concat(const std::string_view& s) const { return z::keyexpr_concat(*this, s); }

inline z::KeyExpr z::KeyExpr::join(const z::KeyExprView& other) const { return z::keyexpr_join(*this, other); }

inline bool z::KeyExpr::equals(const z::KeyExprView& other) const { return z::keyexpr_equals(*this, other); }

inline bool z::KeyExpr::includes(const z::KeyExprView& other) const { return z::keyexpr_includes(*this, other); }

inline bool z::KeyExpr::intersects(const z::KeyExprView& other) const { return z::keyexpr_intersects(*this, other); }
#endif

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
    error = ::z_keyexpr_is_canon(s.data(), s.length());
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
        return config;
    } else {
        return "Failed to create config from file";
    }
}

inline std::variant<z::Config, ErrorMessage> config_from_str(const char* s) {
    z::Config config(::zc_config_from_str(s));
    if (config.check()) {
        return config;
    } else {
        return "Failed to create config from string";
    }
}

inline std::variant<z::Config, ErrorMessage> config_client(const z::StrArrayView& peers) {
    z::Config config(::z_config_client(peers.val, peers.len));
    if (config.check()) {
        return config;
    } else {
        return "Failed to create config from list of peers";
    }
}

inline std::variant<z::Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers) {
    std::vector<const char*> v(peers);
    return z::config_client(v);
}

inline z::ShmManager::ShmManager(const z::Session& session, const char* id, uintptr_t size)
    : Owned(::zc_shm_manager_new(session.loan(), id, size)) {}

inline std::variant<z::ShmManager, z::ErrorMessage> shm_manager_new(const z::Session& session, const char* id,
                                                                    uintptr_t size) {
    z::ShmManager shm_manager(session, id, size);
    if (!shm_manager.check()) return "Failed to create shm manager";
    return shm_manager;
}

inline std::variant<z::Shmbuf, z::ErrorMessage> z::ShmManager::alloc(uintptr_t capacity) const {
    auto shmbuf = z::Shmbuf(::zc_shm_alloc(&_0, capacity));
    if (!shmbuf.check()) return "Failed to allocate shared memor buffer";
    return shmbuf;
}

#endif

inline z::ScoutingConfig z::Config::create_scouting_config() {
    return z::ScoutingConfig(::z_scouting_config_from(loan()));
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

#ifdef __ZENOHCXX_ZENOHC

inline bool z::Publisher::put_owned(z::Payload&& payload, const z::PublisherPutOptions& options, ErrNo& error) {
    return put_owned_impl(std::move(payload), &options, error);
}

inline bool z::Publisher::put_owned(z::Payload&& payload, ErrNo& error) {
    return put_owned_impl(std::move(payload), nullptr, error);
}

inline bool z::Publisher::put_owned(z::Payload&& payload, const z::PublisherPutOptions& options) {
    ErrNo error;
    return put_owned_impl(std::move(payload), &options, error);
}

inline bool z::Publisher::put_owned(z::Payload&& payload) {
    ErrNo error;
    return put_owned_impl(std::move(payload), nullptr, error);
}

#endif

inline bool z::Publisher::put_impl(const z::BytesView& payload, const PublisherPutOptions* options, ErrNo& error) {
    error = ::z_publisher_put(loan(), payload.start, payload.len, options);
    return error == 0;
}

inline bool z::Publisher::delete_impl(const PublisherDeleteOptions* options, ErrNo& error) {
    error = ::z_publisher_delete(loan(), options);
    return error == 0;
}

#ifdef __ZENOHCXX_ZENOHC

inline bool z::Publisher::put_owned_impl(z::Payload&& payload, const z::PublisherPutOptions* options, ErrNo& error) {
    auto p = payload.take();
    error = ::zc_publisher_put_owned(loan(), z_move(p), options);
    return error == 0;
}

#endif

inline bool scout(z::ScoutingConfig&& config, ClosureHello&& callback, ErrNo& error) {
    auto c = config.take();
    auto cb = callback.take();
    error = ::z_scout(z_move(c), z_move(cb));
    return error == 0;
}

inline bool scout(z::ScoutingConfig&& config, ClosureHello&& callback) {
    ErrNo error;
    return z::scout(std::move(config), std::move(callback), error);
}

inline z::KeyExpr z::Session::declare_keyexpr(const z::KeyExprView& keyexpr) {
    return z::KeyExpr(::z_declare_keyexpr(loan(), keyexpr));
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

#ifdef __ZENOHCXX_ZENOHC
inline bool z::Session::put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const PutOptions& options,
                                  ErrNo& error) {
    return put_owned_impl(keyexpr, std::move(payload), &options, error);
}
inline bool z::Session::put_owned(z::KeyExprView keyexpr, z::Payload&& payload, const PutOptions& options) {
    ErrNo error;
    return put_owned_impl(keyexpr, std::move(payload), &options, error);
}
inline bool z::Session::put_owned(z::KeyExprView keyexpr, z::Payload&& payload, ErrNo& error) {
    return put_owned_impl(keyexpr, std::move(payload), nullptr, error);
}
inline bool z::Session::put_owned(z::KeyExprView keyexpr, z::Payload&& payload) {
    ErrNo error;
    PutOptions options;
    return put_owned_impl(keyexpr, std::move(payload), nullptr, error);
}
#endif

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
    error = ::z_info_routers_zid(loan(), z_move(c));
    return error == 0;
}
inline bool z::Session::info_routers_zid(ClosureZid&& callback) {
    auto c = callback.take();
    return ::z_info_routers_zid(loan(), z_move(c)) == 0;
}

inline bool z::Session::info_peers_zid(ClosureZid&& callback, ErrNo& error) {
    auto c = callback.take();
    error = ::z_info_peers_zid(loan(), z_move(c));
    return error == 0;
}
inline bool z::Session::info_peers_zid(ClosureZid&& callback) {
    auto c = callback.take();
    return ::z_info_peers_zid(loan(), z_move(c)) == 0;
}

inline bool z::Session::undeclare_keyexpr_impl(KeyExpr&& keyexpr, ErrNo& error) {
    auto k = keyexpr.take();
    error = ::z_undeclare_keyexpr(loan(), z_move(k));
    return error == 0;
}

inline bool z::Session::get_impl(z::KeyExprView keyexpr, const char* parameters, ClosureReply&& callback,
                                 const GetOptions* options, ErrNo& error) {
    auto c = callback.take();
    error = ::z_get(loan(), keyexpr, parameters, z_move(c), options);
    return error == 0;
}

inline bool z::Session::put_impl(z::KeyExprView keyexpr, const z::BytesView& payload, const PutOptions* options,
                                 ErrNo& error) {
    error = ::z_put(loan(), keyexpr, payload.start, payload.len, options);
    return error == 0;
}

inline bool z::Session::delete_impl(z::KeyExprView keyexpr, const DeleteOptions* options, ErrNo& error) {
    error = ::z_delete(loan(), keyexpr, options);
    return error == 0;
}

#ifdef __ZENOHCXX_ZENOHC
inline bool z::Session::put_owned_impl(z::KeyExprView keyexpr, z::Payload&& payload, const PutOptions* options,
                                       ErrNo& error) {
    auto p = payload.take();
    error = ::zc_put_owned(loan(), keyexpr, z_move(p), options);
    return error == 0;
}
#endif

inline std::variant<z::Queryable, ErrorMessage> z::Session::declare_queryable_impl(z::KeyExprView keyexpr,
                                                                                   ClosureQuery&& callback,
                                                                                   const QueryableOptions* options) {
    auto c = callback.take();
    z::Queryable queryable(::z_declare_queryable(loan(), keyexpr, z_move(c), options));
    if (queryable.check()) {
        return queryable;
    } else {
        return "Unable to create queryable";
    }
}

inline std::variant<z::Subscriber, ErrorMessage> z::Session::declare_subscriber_impl(z::KeyExprView keyexpr,
                                                                                     ClosureSample&& callback,
                                                                                     const SubscriberOptions* options) {
    auto c = callback.take();
    z::Subscriber subscriber(::z_declare_subscriber(loan(), keyexpr, z_move(c), options));
    if (subscriber.check()) {
        return subscriber;
    } else {
        return "Unable to create subscriber";
    }
}

inline std::variant<z::PullSubscriber, ErrorMessage> z::Session::declare_pull_subscriber_impl(
    z::KeyExprView keyexpr, ClosureSample&& callback, const PullSubscriberOptions* options) {
    auto c = callback.take();
    z::PullSubscriber pull_subscriber(::z_declare_pull_subscriber(loan(), keyexpr, z_move(c), options));
    if (pull_subscriber.check()) {
        return pull_subscriber;
    } else {
        return "Unable to create pull subscriber";
    }
}

inline std::variant<z::Publisher, ErrorMessage> z::Session::declare_publisher_impl(z::KeyExprView keyexpr,
                                                                                   const PublisherOptions* options) {
    z::Publisher publisher(::z_declare_publisher(loan(), keyexpr, options));
    if (publisher.check()) {
        return publisher;
    } else {
        return "Unable to create publisher";
    }
}

inline ::z_owned_session_t z::Session::_z_open(z::Config&& v) {
    auto config = v.take();
    return ::z_open(z_move(config));
}

inline std::variant<z::Session, z::ErrorMessage> open(z::Config&& config, bool
#ifdef __ZENOHCXX_ZENOHPICO
                                                                              start_background_tasks
#endif
) {
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
    return session;
}

#ifdef __ZENOHCXX_ZENOHPICO
inline z::Session&& z::Session::operator=(Session&& other) {
    if (this != &other) {
        drop();
        _0 = other._0;
        ::z_null(other._0);
    }
    return std::move(*this);
}

inline void z::Session::drop() {
    if (check()) {
        stop_read_task();
        stop_lease_task();
    }
    Owned::drop();
}
#endif

#ifdef __ZENOHCXX_ZENOHPICO

inline bool z::Config::insert(uint8_t key, const char* value) {
    ErrNo error;
    return insert(key, std::move(value), error);
}

inline bool z::Config::insert(uint8_t key, const char* value, ErrNo& error) {
    error = ::zp_config_insert(loan(), key, ::z_string_make(value));
    return error == 0;
}

#endif

#ifdef __ZENOHCXX_ZENOHPICO
inline bool z::Session::start_read_task() {
    ErrNo error;
    return start_read_task(error);
}
inline bool z::Session::start_read_task(ErrNo& error) {
    error = ::zp_start_read_task(loan(), nullptr);
    return error == 0;
}
inline bool z::Session::stop_read_task() {
    ErrNo error;
    return stop_read_task(error);
}
inline bool z::Session::stop_read_task(ErrNo& error) {
    error = ::zp_stop_read_task(loan());
    return error == 0;
}
inline bool z::Session::start_lease_task() {
    ErrNo error;
    return start_lease_task(error);
}
inline bool z::Session::start_lease_task(ErrNo& error) {
    error = ::zp_start_lease_task(loan(), nullptr);
    return error == 0;
}
inline bool z::Session::stop_lease_task() {
    ErrNo error;
    return stop_lease_task(error);
}
inline bool z::Session::stop_lease_task(ErrNo& error) {
    error = ::zp_stop_lease_task(loan());
    return error == 0;
}
inline bool z::Session::read() {
    ErrNo error;
    return read(error);
}
inline bool z::Session::read(ErrNo& error) {
    error = ::zp_read(loan(), nullptr);
    return error == 0;
}
inline bool z::Session::send_keep_alive() {
    ErrNo error;
    return send_keep_alive(error);
}
inline bool z::Session::send_keep_alive(ErrNo& error) {
    error = ::zp_send_keep_alive(loan(), nullptr);
    return error == 0;
}
inline bool z::Session::send_join() {
    ErrNo error;
    return send_join(error);
}
inline bool z::Session::send_join(ErrNo& error) {
    error = ::zp_send_join(loan(), nullptr);
    return error == 0;
}
#endif

#ifdef __ZENOHCXX_ZENOHPICO
// If the source is successfully resolved, i.e. it was keyexpression declared in the session and call to ``resolve``
// function successfully returned keyexpr string representation, then the resolved keyexpr is returned. Otherwise, the
// original source is returned.
class _Resolved {
   public:
    _Resolved(const z::Session& s, const z::KeyExprView& source)
        : str(source.resolve(s)), keyexpr(str.check() ? z::KeyExprView(str.c_str(), KeyExprUnchecked()) : source) {}
    operator const z::KeyExprView&() const { return keyexpr; }

   private:
    z::Str str;
    const z::KeyExprView keyexpr;
};
#else
class _Resolved {
   public:
    _Resolved(const z::Session&, const z::KeyExprView& source) : keyexpr(source) {}
    operator const z::KeyExprView&() const { return keyexpr; }

   private:
    const z::KeyExprView& keyexpr;
};
#endif

inline bool z::Session::keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::keyexpr_equals(_Resolved(*this, a), _Resolved(*this, b), error);
}

inline bool z::Session::keyexpr_equals(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::Session::keyexpr_equals(a, b, error);
}

inline bool z::Session::keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::keyexpr_includes(_Resolved(*this, a), _Resolved(*this, b), error);
}

inline bool z::Session::keyexpr_includes(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::Session::keyexpr_includes(a, b, error);
}

inline bool z::Session::keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b, ErrNo& error) {
    return z::keyexpr_intersects(_Resolved(*this, a), _Resolved(*this, b), error);
}

inline bool z::Session::keyexpr_intersects(const z::KeyExprView& a, const z::KeyExprView& b) {
    ErrNo error;
    return z::Session::keyexpr_intersects(a, b, error);
}

#ifdef __ZENOHCXX_ZENOHC
inline bool AttachmentView::iterate(const AttachmentView::IterBody& body) const {
    auto cbody = [](struct z_bytes_t key, struct z_bytes_t value, void* context) -> int8_t {
        return (*(static_cast<const IterBody*>(context)))(key, value) == 0;
    };
    return ::z_attachment_iterate(*this, cbody, const_cast<IterBody*>(&body)) == 0;
}
#endif  // ifdef __ZENOHCXX_ZENOHC
