#pragma once

#include "zenoh.h"
#include "zenohcpp_base.h"
#include "zenohcpp_structs.h"
#include <variant>

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
    bool insert_json(const char* key, const char* value) 
        { return zc_config_insert_json(::z_config_loan(&_0), key, value) == 0; }
};

struct Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;
    bool is_ok() const { return ::z_reply_is_ok(&_0); }
    std::variant<Sample,ErrorMessage> get() const {
        if (is_ok()) {
            return Sample{::z_reply_ok(&_0)}; 
        } else {
            return ErrorMessage{::z_reply_err(&_0)}; 
        }
    }
};

typedef Closure<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;

class Session : public Owned<::z_owned_session_t> {
public:
    using Owned::Owned;

    friend std::variant<Session,ErrorMessage> open(Config&& config);

    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options, ErrNo& error) 
        { return get_impl(keyexpr, parameters, std::move(callback), options, error); }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) 
        { ErrNo error; return get_impl(keyexpr, parameters, std::move(callback), options, error); }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback) 
        { ErrNo error; GetOptions options; return get_impl(keyexpr, parameters, std::move(callback), options, error); }

    bool put(KeyExprView keyexpr, const Bytes& payload, const PutOptions& options, ErrNo& error) 
        { return put_impl(keyexpr, payload, options, error); }
    bool put(KeyExprView keyexpr, const Bytes& payload, const PutOptions& options) 
        { ErrNo error; return put_impl(keyexpr, payload, options, error); }
    bool put(KeyExprView keyexpr, const Bytes& payload) 
        { ErrNo error; PutOptions options; return put_impl(keyexpr, payload, options, error); }

private:

    bool get_impl(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options, ErrNo& error) { 
        auto c = callback.take(); 
        error = ::z_get(::z_session_loan(&_0), keyexpr, parameters, &c, &options); 
        return error == 0;
    }

    bool put_impl(KeyExprView keyexpr, const Bytes& payload, const PutOptions& options, ErrNo& error) { 
        error = ::z_put(::z_session_loan(&_0), keyexpr, payload.start, payload.len, &options); 
        return error == 0;
    }

    Session(Config&& v) : Owned(_z_open(std::move(v))) {} 
    static ::z_owned_session_t _z_open(Config&& v) {
        auto config = v.take();
        return ::z_open(z_move(config));
    };
};

std::variant<Session,ErrorMessage> open(Config&& config) {
    Session session(std::move(config));
    if (session.check()) {
        return std::move(session);
    } else {
        return "Unable to open session";
    }
};

}