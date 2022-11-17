#pragma once

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
    bool insert_json(const char* key, const char* value) 
        { return zc_config_insert_json(::z_config_loan(&_0), key, value) == 0; }
};

struct Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;
    bool is_ok() const { return ::z_reply_is_ok(&_0); }
    Sample ok() const { return Sample{::z_reply_ok(&_0)}; }
};

typedef Closure<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;

class Session : public Owned<::z_owned_session_t> {
public:
    using Owned::Owned;
    explicit Session(Config&& v) : Owned(_z_open(std::move(v))) {} 
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) 
        { auto c = callback.take(); return z_get(z_session_loan(&_0), keyexpr, parameters, &c, &options); }
private:
    static ::z_owned_session_t _z_open(Config&& v) {
        auto config = v.take();
        return ::z_open(z_move(config));
    };
};


}