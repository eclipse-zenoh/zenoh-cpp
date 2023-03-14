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

// Convenient representation of owned strings returned from zenoh-c
// which are supposed to be freed by user
class Str : public Owned<::z_owned_str_t> {
   public:
    using Owned::Owned;
    operator const char*() const { return ::z_loan(_0); }
    const char* c_str() const { return ::z_loan(_0); }
    bool operator==(const std::string_view& s) const { return s == c_str(); }
    bool operator==(const char* s) const { return std::string_view(s) == c_str(); }
};

class KeyExpr : public Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;
    explicit KeyExpr(nullptr_t) : Owned(nullptr) {}
    explicit KeyExpr(const char* name) : Owned(::z_keyexpr_new(name)) {}
    KeyExprView as_keyexpr_view() const { return KeyExprView(::z_keyexpr_loan(&_0)); }
    operator KeyExprView() const { return as_keyexpr_view(); }
    BytesView as_bytes() const { return as_keyexpr_view().as_bytes(); }
    std::string_view as_string_view() const { return as_keyexpr_view().as_string_view(); }
    bool operator==(const std::string_view& v) { return as_string_view() == v; }
#ifdef __ZENOHCXX_ZENOHC
    KeyExpr concat(const std::string_view& s) const { return as_keyexpr_view().concat(s); }
    KeyExpr join(const KeyExprView& v) const { return as_keyexpr_view().join(v); }
#endif
    bool equals(const KeyExprView& v) const { return as_keyexpr_view().equals(v); }
    bool includes(const KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().includes(v, error); }
    bool includes(const KeyExprView& v) const { return as_keyexpr_view().includes(v); }
    bool intersects(const KeyExprView& v, ErrNo& error) const { return as_keyexpr_view().intersects(v, error); }
    bool intersects(const KeyExprView& v) const { return as_keyexpr_view().intersects(v); }
};

#ifdef __ZENOHCXX_ZENOHC
KeyExpr KeyExprView::concat(const std::string_view& s) const { return ::z_keyexpr_concat(*this, s.data(), s.length()); }
KeyExpr KeyExprView::join(const KeyExprView& v) const { return ::z_keyexpr_join(*this, v); }
#endif

class ScoutingConfig;

class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
#ifdef __ZENOHCXX_ZENOHC
    Str get(const char* key) const { return Str(::zc_config_get(::z_config_loan(&_0), key)); }
    Str to_string() const { return Str(::zc_config_to_string(::z_config_loan(&_0))); }
    bool insert_json(const char* key, const char* value) {
        return ::zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
#endif
#ifdef __ZENOHCXX_ZENOHPICO
    const char* get(uint8_t key) const { return ::zp_config_get(::z_config_loan(&_0), key); }
#endif
    ScoutingConfig create_scouting_config();
};

#ifdef __ZENOHCXX_ZENOHC
Config config_peer() { return Config(::z_config_peer()); }

std::variant<Config, ErrorMessage> config_from_file(const char* path) {
    Config config(::zc_config_from_file(path));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from file";
    }
}

std::variant<Config, ErrorMessage> config_from_str(const char* s) {
    Config config(::zc_config_from_str(s));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from string";
    }
}

std::variant<Config, ErrorMessage> config_client(const StrArrayView& peers) {
    Config config(::z_config_client(peers.val, peers.len));
    if (config.check()) {
        return std::move(config);
    } else {
        return "Failed to create config from list of peers";
    }
}

std::variant<Config, ErrorMessage> config_client(const std::initializer_list<const char*>& peers) {
    std::vector<const char*> v(peers);
    return config_client(v);
}
#endif

class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
    ScoutingConfig(Config& config) : Owned(std::move(ScoutingConfig(config))) {}
};

inline ScoutingConfig Config::create_scouting_config() {
    return ScoutingConfig(::z_scouting_config_from(::z_loan(_0)));
}

class Reply : public Owned<::z_owned_reply_t> {
   public:
    using Owned::Owned;
    bool is_ok() const { return ::z_reply_is_ok(&_0); }
    std::variant<Sample, ErrorMessage> get() const {
        if (is_ok()) {
            return Sample{::z_reply_ok(&_0)};
        } else {
            return ErrorMessage{::z_reply_err(&_0)};
        }
    }
};

class Subscriber : public Owned<::z_owned_subscriber_t> {
   public:
    using Owned::Owned;
};

class PullSubscriber : public Owned<::z_owned_pull_subscriber_t> {
   public:
    using Owned::Owned;
    bool pull() { return ::z_subscriber_pull(::z_loan(_0)) == 0; }
    bool pull(ErrNo& error) {
        error = ::z_subscriber_pull(::z_loan(_0));
        return error == 0;
    }
};

class Queryable : public Owned<::z_owned_queryable_t> {
   public:
    using Owned::Owned;
};

class Publisher : public Owned<::z_owned_publisher_t> {
   public:
    using Owned::Owned;
    bool put(const BytesView& payload, const PublisherPutOptions& options, ErrNo& error) {
        return put_impl(payload, &options, error);
    }
    bool put(const BytesView& payload, ErrNo& error) { return put_impl(payload, nullptr, error); }
    bool put(const BytesView& payload, const PublisherPutOptions& options) {
        ErrNo error;
        return put_impl(payload, &options, error);
    }
    bool put(const BytesView& payload) {
        ErrNo error;
        return put_impl(payload, nullptr, error);
    }

    bool delete_resource(const PublisherDeleteOptions& options, ErrNo& error) { return delete_impl(&options, error); }
    bool delete_resource(ErrNo& error) { return delete_impl(nullptr, error); }
    bool delete_resource(const PublisherDeleteOptions& options) {
        ErrNo error;
        return delete_impl(&options, error);
    }
    bool delete_resource() {
        ErrNo error;
        return delete_impl(nullptr, error);
    }

   private:
    bool put_impl(const BytesView& payload, const PublisherPutOptions* options, ErrNo& error) {
        error = ::z_publisher_put(::z_loan(_0), payload.start, payload.len, options);
        return error == 0;
    }

    bool delete_impl(const PublisherDeleteOptions* options, ErrNo& error) {
        error = ::z_publisher_delete(::z_loan(_0), options);
        return error == 0;
    }
};

class Hello : public Owned<::z_owned_hello_t> {
   public:
    using Owned::Owned;
    operator HelloView() const { return HelloView(::z_hello_loan(&_0)); }
};

typedef ClosureMoveParam<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;

typedef ClosureConstPtrParam<::z_owned_closure_query_t, ::z_query_t, Query> ClosureQuery;

typedef ClosureConstPtrParam<::z_owned_closure_sample_t, ::z_sample_t, Sample> ClosureSample;

typedef ClosureConstPtrParam<::z_owned_closure_zid_t, ::z_id_t, Id> ClosureZid;

typedef ClosureMoveParam<::z_owned_closure_hello_t, ::z_owned_hello_t, Hello> ClosureHello;

bool scout(ScoutingConfig&& config, ClosureHello&& callback, ErrNo& error) {
    auto c = config.take();
    auto cb = callback.take();
    error = ::z_scout(z_move(c), z_move(cb));
    return error == 0;
};

bool scout(ScoutingConfig&& config, ClosureHello&& callback) {
    ErrNo error;
    return scout(std::move(config), std::move(callback), error);
}

class Session : public Owned<::z_owned_session_t> {
   public:
    using Owned::Owned;

    Id info_zid() const { return ::z_info_zid(::z_session_loan(&_0)); }

    friend std::variant<Session, ErrorMessage> open(Config&& config);

    KeyExpr declare_keyexpr(const KeyExprView& keyexpr) {
        return KeyExpr(::z_declare_keyexpr(::z_session_loan(&_0), keyexpr));
    }

    bool undeclare_keyexpr(KeyExpr&& keyexpr, ErrNo& error) {
        return undeclare_keyexpr_impl(std::move(keyexpr), error);
    }

    bool undeclare_keyexpr(KeyExpr&& keyexpr) {
        ErrNo error;
        return undeclare_keyexpr_impl(std::move(keyexpr), error);
    }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options,
             ErrNo& error) {
        return get_impl(keyexpr, parameters, std::move(callback), &options, error);
    }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) {
        ErrNo error;
        return get_impl(keyexpr, parameters, std::move(callback), &options, error);
    }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, ErrNo& error) {
        return get_impl(keyexpr, parameters, std::move(callback), nullptr, error);
    }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback) {
        ErrNo error;
        return get_impl(keyexpr, parameters, std::move(callback), nullptr, error);
    }

    bool put(KeyExprView keyexpr, const BytesView& payload, const PutOptions& options, ErrNo& error) {
        return put_impl(keyexpr, payload, &options, error);
    }
    bool put(KeyExprView keyexpr, const BytesView& payload, const PutOptions& options) {
        ErrNo error;
        return put_impl(keyexpr, payload, &options, error);
    }
    bool put(KeyExprView keyexpr, const BytesView& payload, ErrNo& error) {
        return put_impl(keyexpr, payload, nullptr, error);
    }
    bool put(KeyExprView keyexpr, const BytesView& payload) {
        ErrNo error;
        PutOptions options;
        return put_impl(keyexpr, payload, nullptr, error);
    }

    bool delete_resource(KeyExprView keyexpr, const DeleteOptions& options, ErrNo& error) {
        return delete_impl(keyexpr, &options, error);
    }
    bool delete_resource(KeyExprView keyexpr, const DeleteOptions& options) {
        ErrNo error;
        return delete_impl(keyexpr, &options, error);
    }
    bool delete_resource(KeyExprView keyexpr, ErrNo& error) { return delete_impl(keyexpr, nullptr, error); }
    bool delete_resource(KeyExprView keyexpr) {
        ErrNo error;
        PutOptions options;
        return delete_impl(keyexpr, nullptr, error);
    }

    std::variant<Queryable, ErrorMessage> declare_queryable(KeyExprView keyexpr, ClosureQuery&& callback,
                                                            const QueryableOptions& options) {
        return declare_queryable_impl(keyexpr, std::move(callback), &options);
    }
    std::variant<Queryable, ErrorMessage> declare_queryable(KeyExprView keyexpr, ClosureQuery&& callback) {
        return declare_queryable_impl(keyexpr, std::move(callback), nullptr);
    }

    std::variant<Subscriber, ErrorMessage> declare_subscriber(KeyExprView keyexpr, ClosureSample&& callback,
                                                              const SubscriberOptions& options) {
        return declare_subscriber_impl(keyexpr, std::move(callback), &options);
    }
    std::variant<Subscriber, ErrorMessage> declare_subscriber(KeyExprView keyexpr, ClosureSample&& callback) {
        return declare_subscriber_impl(keyexpr, std::move(callback), nullptr);
    }

    std::variant<PullSubscriber, ErrorMessage> declare_pull_subscriber(KeyExprView keyexpr, ClosureSample&& callback,
                                                                       const PullSubscriberOptions& options) {
        return declare_pull_subscriber_impl(keyexpr, std::move(callback), &options);
    }
    std::variant<PullSubscriber, ErrorMessage> declare_pull_subscriber(KeyExprView keyexpr, ClosureSample&& callback) {
        return declare_pull_subscriber_impl(keyexpr, std::move(callback), nullptr);
    }

    std::variant<Publisher, ErrorMessage> declare_publisher(KeyExprView keyexpr, const PublisherOptions& options) {
        return declare_publisher_impl(keyexpr, &options);
    }
    std::variant<Publisher, ErrorMessage> declare_publisher(KeyExprView keyexpr) {
        return declare_publisher_impl(keyexpr, nullptr);
    }

    bool info_routers_zid(ClosureZid&& callback, ErrNo& error) {
        auto c = callback.take();
        error = ::z_info_routers_zid(::z_session_loan(&_0), &c);
        return error == 0;
    }
    bool info_routers_zid(ClosureZid&& callback) {
        auto c = callback.take();
        return ::z_info_routers_zid(::z_session_loan(&_0), &c) == 0;
    }

    bool info_peers_zid(ClosureZid&& callback, ErrNo& error) {
        auto c = callback.take();
        error = ::z_info_peers_zid(::z_session_loan(&_0), &c);
        return error == 0;
    }
    bool info_peers_zid(ClosureZid&& callback) {
        auto c = callback.take();
        return ::z_info_peers_zid(::z_session_loan(&_0), &c) == 0;
    }

   private:
    bool undeclare_keyexpr_impl(KeyExpr&& keyexpr, ErrNo& error) {
        error = ::z_undeclare_keyexpr(::z_session_loan(&_0), &(static_cast<::z_owned_keyexpr_t&>(keyexpr)));
        return error == 0;
    }

    bool get_impl(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions* options,
                  ErrNo& error) {
        auto c = callback.take();
        error = ::z_get(::z_session_loan(&_0), keyexpr, parameters, &c, options);
        return error == 0;
    }

    bool put_impl(KeyExprView keyexpr, const BytesView& payload, const PutOptions* options, ErrNo& error) {
        error = ::z_put(::z_session_loan(&_0), keyexpr, payload.start, payload.len, options);
        return error == 0;
    }

    bool delete_impl(KeyExprView keyexpr, const DeleteOptions* options, ErrNo& error) {
        error = ::z_delete(::z_session_loan(&_0), keyexpr, options);
        return error == 0;
    }
    std::variant<Queryable, ErrorMessage> declare_queryable_impl(KeyExprView keyexpr, ClosureQuery&& callback,
                                                                 const QueryableOptions* options) {
        auto c = callback.take();
        Queryable queryable(::z_declare_queryable(::z_session_loan(&_0), keyexpr, &c, options));
        if (queryable.check()) {
            return std::move(queryable);
        } else {
            return "Unable to create queryable";
        }
    }

    std::variant<Subscriber, ErrorMessage> declare_subscriber_impl(KeyExprView keyexpr, ClosureSample&& callback,
                                                                   const SubscriberOptions* options) {
        auto c = callback.take();
        Subscriber subscriber(::z_declare_subscriber(::z_session_loan(&_0), keyexpr, &c, options));
        if (subscriber.check()) {
            return std::move(subscriber);
        } else {
            return "Unable to create subscriber";
        }
    }

    std::variant<PullSubscriber, ErrorMessage> declare_pull_subscriber_impl(KeyExprView keyexpr,
                                                                            ClosureSample&& callback,
                                                                            const PullSubscriberOptions* options) {
        auto c = callback.take();
        PullSubscriber pull_subscriber(::z_declare_pull_subscriber(::z_session_loan(&_0), keyexpr, &c, options));
        if (pull_subscriber.check()) {
            return std::move(pull_subscriber);
        } else {
            return "Unable to create pull subscriber";
        }
    }

    std::variant<Publisher, ErrorMessage> declare_publisher_impl(KeyExprView keyexpr, const PublisherOptions* options) {
        Publisher publisher(::z_declare_publisher(::z_session_loan(&_0), keyexpr, options));
        if (publisher.check()) {
            return std::move(publisher);
        } else {
            return "Unable to create publisher";
        }
    }

    Session(Config&& v) : Owned(_z_open(std::move(v))) {}
    static ::z_owned_session_t _z_open(Config&& v) {
        auto config = v.take();
        return ::z_open(z_move(config));
    }
};

std::variant<Session, ErrorMessage> open(Config&& config) {
    Session session(std::move(config));
    if (session.check()) {
        return std::move(session);
    } else {
        return "Unable to open session";
    }
};