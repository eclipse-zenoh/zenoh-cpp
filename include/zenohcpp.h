#pragma once

#include "zenoh.h"
#include <utility>

namespace zenoh {

typedef z_keyexpr_t KeyExpr;
typedef z_bytes_t Bytes;

inline KeyExpr keyexpr(const char* keyexpr) { return z_keyexpr(keyexpr); }
inline bool check(const KeyExpr& keyexpr) { return z_keyexpr_is_initialized(&keyexpr); }
inline Bytes as_bytes(KeyExpr keyexpr) { return z_keyexpr_as_bytes(keyexpr); }

class OwnedClosureReply {
public:
    OwnedClosureReply() = delete;
    OwnedClosureReply& operator=(const OwnedClosureReply&) = delete;
    OwnedClosureReply(const OwnedClosureReply&) = delete;
    OwnedClosureReply(z_owned_closure_reply_t&& v) : _0(v) { v = {}; }
    virtual ~OwnedClosureReply() { z_closure_reply_drop(&_0); }
private:
    z_owned_closure_reply_t _0;

    friend class OwnedSession;
};


class GetOptions {
public:
    GetOptions() : _0(z_get_options_default()) {}
    GetOptions& target(z_query_target_t v) { _0.target = v; return *this; }
    operator z_get_options_t&() { return _0; }
private:
    z_get_options_t _0;
};

class OwnedConfig {
public:
    OwnedConfig& operator=(const OwnedConfig& v) = delete;
    OwnedConfig(const OwnedConfig& v) = delete;
    OwnedConfig() : _0(z_config_default()) {}
    explicit OwnedConfig(z_owned_config_t&& v) : _0(v) { v = {}; }
    explicit OwnedConfig(OwnedConfig&& v) : OwnedConfig(std::move(v._0)) {}
    virtual ~OwnedConfig() { z_config_drop(&_0); }

    z_owned_config_t take() { auto _ = _0; _0 = {}; return _; }
    operator const z_owned_config_t&() const { return _0; }
    bool insert_json(const char* key, const char* value) 
        { return zc_config_insert_json(z_config_loan(&_0), key, value) == 0; }

private:
    z_owned_config_t _0;

    friend class OwnedSession;
};

class OwnedSession {
public:
    OwnedSession& operator=(const OwnedSession&) = delete;
    OwnedSession(const OwnedSession&) = delete;
    OwnedSession() = delete;
    explicit OwnedSession(z_owned_session_t&& v) : _0(v) { v ={}; }
    explicit OwnedSession(OwnedSession&& v) : OwnedSession(std::move(v._0)) {}
    explicit OwnedSession(OwnedConfig&& v) : _0(z_open(z_move(v._0))) {}
    virtual ~OwnedSession() { z_close(&_0); }

    bool check() const { return z_session_check(&_0); }
    bool get(KeyExpr keyexpr, const char* parameters, OwnedClosureReply&& callback, const z_get_options_t& options) 
        { return z_get(z_session_loan(&_0), keyexpr, parameters, z_move(callback._0), &options ); }
private:
    z_owned_session_t _0;
};

class OwnedReplyChannel {
public:
    OwnedReplyChannel& operator=(const OwnedReplyChannel&) = delete;
    OwnedReplyChannel(const OwnedReplyChannel&) = delete;
    OwnedReplyChannel() = delete;
    OwnedReplyChannel(z_owned_reply_channel_t&& v) : _0(v) { v = {}; }
    virtual ~OwnedReplyChannel() { z_reply_channel_drop(&_0); }

    OwnedClosureReply take_send() { return OwnedClosureReply(std::move(_0.send)); }
    
    const struct z_owned_reply_channel_closure_t& recv() const { return _0.recv; }

private:
    z_owned_reply_channel_t _0;
};

OwnedReplyChannel reply_fifo_new(uintptr_t bound)
{
    return OwnedReplyChannel(std::move(zc_reply_fifo_new(bound)));
}


}
