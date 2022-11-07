#pragma once

#include "zenoh.h"
#include <utility>
#include <string>

namespace zenoh {

inline const char* to_const_char_ptr(const uint8_t* buf) { return reinterpret_cast<const char*>(buf); };

struct KeyExprRef : public z_keyexpr_t {
    KeyExprRef(const char* expr) : z_keyexpr_t(z_keyexpr(expr)) {}
    bool check() const { return z_keyexpr_is_initialized(this); }
    std::string str() const {
        auto bytes = z_keyexpr_as_bytes(*this);
        return std::string(to_const_char_ptr(bytes.start), bytes.len);
    }
};

struct Config : public z_owned_config_t {
    Config() : z_owned_config_t(z_config_default()) {}
    virtual ~Config() { z_config_drop(this); }
    bool insert_json(const char* key, const char* value) { return zc_config_insert_json(z_config_loan(this), key, value) == 0;}
};

struct Sample : public z_sample_t {
    const KeyExprRef& get_keyexpr() const { return static_cast<const KeyExprRef&>(keyexpr); }
};

struct Reply : public z_owned_reply_t {
    Reply() : z_owned_reply_t(z_reply_null()) {}
    virtual ~Reply() { z_reply_drop(this); }
    bool check() const { return z_reply_check(this); }
    bool is_ok() const { return z_reply_is_ok(this); }
    Sample ok() const { return Sample { z_reply_ok(this) }; }
};

struct ClosureReply : public z_owned_closure_reply_t {
    ClosureReply(z_owned_closure_reply_t&& v) : z_owned_closure_reply_t(v) { v = {}; }
    virtual ~ClosureReply() { z_closure_reply_drop(this); }
};

struct ReplyChannelClosure : public z_owned_reply_channel_closure_t {
    bool call(Reply& sample) const { return z_reply_channel_closure_call(this, &sample); }
};

struct ReplyChannel : public z_owned_reply_channel_t {
    ReplyChannel(z_owned_reply_channel_t&& v) : z_owned_reply_channel_t({v.send, v.recv}) { v = {}; }
    ClosureReply take_send() { return ClosureReply(std::move(send)); }
    const ReplyChannelClosure& get_recv() const { return static_cast<const ReplyChannelClosure&>(recv); }
};

struct ReplyFIFO : public ReplyChannel {
    ReplyFIFO(uintptr_t bound) : ReplyChannel(zc_reply_fifo_new(bound)) {}
};

struct GetOptions : public z_get_options_t {
    GetOptions() : z_get_options_t(z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) { target = v; return *this; }
};

struct Session : public z_owned_session_t {
    Session(Config&& config) : z_owned_session_t(z_open(z_move(config))) {}
    bool check() const { return z_session_check(this); }
    bool get(KeyExprRef keyexpr, const char* parameters, ClosureReply&& callback, const GetOptions& options) {
        return z_get(z_session_loan(this), keyexpr, parameters, z_move(callback), &options );
    }
};

}