#pragma once

#include "zenoh.h"
#include "zenohcpp_params.h"

#include <utility>

namespace zenoh {

template<typename ZC_OWNED_TYPE> class Owned {
public:
    Owned& operator=(const Owned& v) = delete;
    Owned(const Owned& v) = delete;
    Owned(ZC_OWNED_TYPE&& v) : _0(v) { v = {}; }
    Owned(Owned&& v) : Owned(std::move(v._0)) {}
    virtual ~Owned() { z_drop(&_0); }
    ZC_OWNED_TYPE take() { auto r = _0; _0 = {}; return r; }
    bool check() { return z_check(_0); }
protected:
    Owned() : _0({}) {};
    ZC_OWNED_TYPE _0;
};

class KeyExprView {
public:
    KeyExprView(const char* name) : _0(z_keyexpr(name)) {}
    explicit KeyExprView(z_keyexpr_t v) : _0(v) {}
    bool check() const { return z_keyexpr_is_initialized(&_0); }
    operator ::z_keyexpr_t() const { return _0; }
private:
    ::z_keyexpr_t _0;
};

class KeyExpr : Owned<::z_owned_keyexpr_t> {
public:
    using Owned::Owned;
    explicit KeyExpr(const char* name) { _0 = z_keyexpr_new(name); }
    operator KeyExprView() const { return KeyExprView(z_keyexpr_loan(&_0)); }
};

class Config : public Owned<::z_owned_config_t> {
public:
    using Owned::Owned;
    Config() { _0 = z_config_default(); }
    bool insert_json(const char* key, const char* value) 
        { return zc_config_insert_json(z_config_loan(&_0), key, value) == 0; }
};

struct Reply : public Owned<::z_owned_reply_t> {
public:
    using Owned::Owned;
    bool is_ok() const { return z_reply_is_ok(&_0); }
};

class Session : public Owned<::z_owned_session_t> {
public:
    using Owned::Owned;
    explicit Session(Config&& v) { 
        auto config = v.take();
        _0 = z_open(z_move(config));
    }
    bool get(KeyExprView keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) 
        { auto c = callback.take(); return z_get(z_session_loan(&_0), keyexpr, parameters, &c, &options); }
};


}