#pragma once

#include <variant>

#include "zenoh.h"
#include "zenohcpp_base.h"
#include "zenohcpp_structs.h"

namespace zenoh {

class KeyExpr : Owned<::z_owned_keyexpr_t> {
   public:
    using Owned::Owned;
    explicit KeyExpr(const char* name) : Owned(::z_keyexpr_new(name)) {}
    operator KeyExprView() const { return KeyExprView(::z_keyexpr_loan(&_0)); }
};

class ScoutingConfig;

class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
    bool insert_json(const char* key, const char* value) {
        return zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
    operator ScoutingConfig();
};

class ScoutingConfig : public Owned<::z_owned_scouting_config_t> {
   public:
    using Owned::Owned;
    ScoutingConfig() : Owned(::z_scouting_config_default()) {}
    ScoutingConfig(Config& config) : Owned(std::move(ScoutingConfig(config))) {}
};

inline Config::operator ScoutingConfig() { return ScoutingConfig(::z_scouting_config_from(::z_loan(_0))); }

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

class PullSubscriber : public Owned<::z_owned_pull_subscriber_t> {
   public:
    using Owned::Owned;
    bool pull() { return z_subscriber_pull(::z_loan(_0)) == 0; }
    bool pull(ErrNo& error) {
        error = z_subscriber_pull(::z_loan(_0));
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
    bool put(const Bytes& payload, const PublisherPutOptions& options, ErrNo& error) {
        return put_impl(payload, &options, error);
    }
    bool put(const Bytes& payload, ErrNo& error) { return put_impl(payload, nullptr, error); }
    bool put(const Bytes& payload, const PublisherPutOptions& options) {
        ErrNo error;
        return put_impl(payload, &options, error);
    }
    bool put(const Bytes& payload) {
        ErrNo error;
        return put_impl(payload, nullptr, error);
    }

   private:
    bool put_impl(const Bytes& payload, const PublisherPutOptions* options, ErrNo& error) {
        error = ::z_publisher_put(::z_loan(_0), payload.start, payload.len, options);
        return error == 0;
    }
};

class Hello : public Owned<::z_owned_hello_t> {
   public:
    using Owned::Owned;
};

typedef Closure<::z_owned_closure_reply_t, ::z_owned_reply_t*, Reply> ClosureReply;

typedef Closure<::z_owned_closure_query_t, const ::z_query_t*, Query> ClosureQuery;

typedef Closure<::z_owned_closure_sample_t, const ::z_sample_t*, Sample> ClosureSample;

typedef Closure<::z_owned_closure_zid_t, const ::z_id_t*, Id> ClosureZid;

typedef Closure<::z_owned_closure_hello_t, ::z_owned_hello_t*, Hello> ClosureHello;

bool scout(ScoutingConfig&& config, ClosureHello&& callback, ErrNo& error) {
    auto c = config.take();
    auto cb = callback.take();
    error = ::z_scout(z_move(c), z_move(cb));
    return error == 0;
};

bool scout(ScoutingConfig&& config, ClosureHello&& callback) {
    ErrNo error;
    return scout(std::move(config), std::move(callback));
}

class Session : public Owned<::z_owned_session_t> {
   public:
    using Owned::Owned;

    Id info_zid() const { return ::z_info_zid(::z_session_loan(&_0)); }

    friend std::variant<Session, ErrorMessage> open(Config&& config);

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

    bool put(KeyExprView keyexpr, const Bytes& payload, const PutOptions& options, ErrNo& error) {
        return put_impl(keyexpr, payload, &options, error);
    }
    bool put(KeyExprView keyexpr, const Bytes& payload, const PutOptions& options) {
        ErrNo error;
        return put_impl(keyexpr, payload, &options, error);
    }
    bool put(KeyExprView keyexpr, const Bytes& payload, ErrNo& error) {
        return put_impl(keyexpr, payload, nullptr, error);
    }
    bool put(KeyExprView keyexpr, const Bytes& payload) {
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
    bool delete_resource(KeyExprView keyexpr, ErrNo& error) {
        return delete_impl(keyexpr, nullptr, error);
    }
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
    bool get_impl(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions* options,
                  ErrNo& error) {
        auto c = callback.take();
        error = ::z_get(::z_session_loan(&_0), keyexpr, parameters, &c, options);
        return error == 0;
    }

    bool put_impl(KeyExprView keyexpr, const Bytes& payload, const PutOptions* options, ErrNo& error) {
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

}  // namespace zenoh