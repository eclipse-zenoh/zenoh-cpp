#pragma once

#include "zenoh.h"
#include "zenohcpp_params.h"

#include <utility>

namespace zenoh {

class KeyExprView {
public:
    KeyExprView(const char* name) : _0(z_keyexpr(name)) {}
    explicit KeyExprView(z_keyexpr_t v) : _0(v) {}

    bool check() const { return z_keyexpr_is_initialized(&_0); }

    operator ::z_keyexpr_t() const { return _0; }

private:
    z_keyexpr_t _0;
};

class KeyExpr {
public:
    explicit KeyExpr(const char* name) : _0(z_keyexpr_new(name)) {}
    KeyExpr(const KeyExpr&) = delete;
    KeyExpr& operator=(const KeyExpr&) = delete;
    virtual ~KeyExpr() { z_keyexpr_drop(&_0); }

    operator KeyExprView() const { return KeyExprView(z_keyexpr_loan(&_0)); }
private:
    z_owned_keyexpr_t _0;
};


class Config {
public:
    Config& operator=(const Config& v) = delete;
    Config(const Config& v) = delete;
    Config() : _0(z_config_default()) {}
    Config(Config&& v) : Config(std::move(v._0)) {}
    virtual ~Config() { z_config_drop(&_0); }

    explicit Config(z_owned_config_t&& v) : _0(v) { v = {}; }
    z_owned_config_t take() { auto r = _0; _0 = {}; return r; }

    bool insert_json(const char* key, const char* value) 
        { return zc_config_insert_json(z_config_loan(&_0), key, value) == 0; }

private:
    z_owned_config_t _0;

    friend class Session;
};

struct Reply {
public:
    Reply& operator=(const Reply& v) = delete;
    Reply(const Reply& v) = delete;
    explicit Reply(z_owned_reply_t&& v) : _0(v) { v = {}; }
    virtual ~Reply() { z_reply_drop(&_0); }
    bool check() const { return z_reply_check(&_0); }
    bool is_ok() const { return z_reply_is_ok(&_0); }
private:
    z_owned_reply_t _0;
};

template<typename ZC_CLOSURE_TYPE, typename ZC_CLOSURE_CALL_PARAM, typename ZCPP_CLOSURE_CALL_PARAM> class Closure {
public:
    template<typename LAMBDA> Closure(LAMBDA&& lambda) {
        zclosure.context = new LAMBDA(std::move(lambda));
        zclosure.drop = [](void* ctx){delete static_cast<LAMBDA*>(ctx);};
        zclosure.call = [](ZC_CLOSURE_CALL_PARAM* value, void* ctx){static_cast<LAMBDA*>(ctx)->operator()(ZCPP_CLOSURE_CALL_PARAM(std::move(*value)));};
    }
    Closure(const Closure&) = delete;
    Closure(Closure&& v) {
        this->zclosure = v.zclosure;
        v.zclosure = {};
    }
    ~Closure() {
        if (zclosure.drop) {
            zclosure.drop(zclosure.context);
        }
    }

    ZC_CLOSURE_TYPE take() { auto v = zclosure; zclosure = {}; return v; }

private:
    ZC_CLOSURE_TYPE zclosure; 
};

typedef Closure<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;

class Session {
public:
    Session& operator=(const Session&) = delete;
    Session(const Session&) = delete;
    Session() = delete;
    explicit Session(z_owned_session_t&& v) : _0(v) { v ={}; }
    explicit Session(Session&& v) : Session(std::move(v._0)) {}
    explicit Session(Config&& v) : _0(z_open(z_move(v._0))) {}
    virtual ~Session() { z_close(&_0); }

    bool check() const { return z_session_check(&_0); }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) 
        { auto c = callback.take(); return z_get(z_session_loan(&_0), keyexpr, parameters, &c, &options); }
private:
    z_owned_session_t _0;

};


}