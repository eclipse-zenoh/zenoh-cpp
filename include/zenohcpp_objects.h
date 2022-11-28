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

class Config : public Owned<::z_owned_config_t> {
   public:
    using Owned::Owned;
    Config() : Owned(::z_config_default()) {}
    bool insert_json(const char* key, const char* value) {
        return zc_config_insert_json(::z_config_loan(&_0), key, value) == 0;
    }
};

struct Reply : public Owned<::z_owned_reply_t> {
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

struct PullSubscriber : public Owned<::z_owned_pull_subscriber_t> {
   public:
    using Owned::Owned;
    bool pull() { return z_subscriber_pull(z_loan(_0)) == 0; }
    bool pull(ErrNo& error) {
        error = z_subscriber_pull(z_loan(_0));
        return error == 0;
    }
};

typedef Closure<::z_owned_closure_reply_t, struct ::z_owned_reply_t*, Reply> ClosureReply;

typedef Closure<::z_owned_closure_query_t, const ::z_query_t*, Query> ClosureQuery;

typedef Closure<::z_owned_closure_sample_t, const ::z_sample_t*, Sample> ClosureSample;

struct Queryable : public Owned<::z_owned_queryable_t> {
   public:
    using Owned::Owned;
};

class Session : public Owned<::z_owned_session_t> {
   public:
    using Owned::Owned;

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

    Session(Config&& v) : Owned(_z_open(std::move(v))) {}
    static ::z_owned_session_t _z_open(Config&& v) {
        auto config = v.take();
        return ::z_open(z_move(config));
    };
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