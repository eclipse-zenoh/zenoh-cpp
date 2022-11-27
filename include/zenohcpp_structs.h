#pragma once

#include <string_view>

#include "string.h"
#include "zenoh.h"

namespace zenoh {

struct Bytes : public ::z_bytes_t {
    Bytes() : ::z_bytes_t({}) {}
    Bytes(::z_bytes_t v) : ::z_bytes_t(v) {}
    Bytes(const char* s) : ::z_bytes_t({start : reinterpret_cast<const uint8_t*>(s), len : strlen(s)}) {}
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start), len); }
};

struct KeyExprView : public ::z_keyexpr_t {
    KeyExprView(const char* name) : ::z_keyexpr_t(z_keyexpr(name)) {}
    KeyExprView(const z_keyexpr_t& v) : ::z_keyexpr_t(v) {}
    bool check() const { return z_keyexpr_is_initialized(this); }
    Bytes as_bytes() const { return Bytes{::z_keyexpr_as_bytes(*this)}; }
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }
};

typedef ::z_encoding_prefix_t EncodingPrefix;

struct Encoding : public ::z_encoding_t {
    Encoding() : ::z_encoding_t(::z_encoding_default()) {}
    Encoding(::z_encoding_t v) : ::z_encoding_t(v) {}
    Encoding(EncodingPrefix _prefix) : ::z_encoding_t(::z_encoding(_prefix, nullptr)) {}
    Encoding(EncodingPrefix _prefix, const char* _suffix) : ::z_encoding_t(::z_encoding(_prefix, _suffix)) {}
};

struct Sample : public ::z_sample_t {
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
};

struct Value : public ::z_value_t {
    Value() : ::z_value_t({}) {}
    Value(::z_value_t v) : ::z_value_t(v) {}
    Value(const char* v) : ::z_value_t({payload : Bytes(v), encoding : Encoding()}) {}
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    std::string_view as_string_view() const { return get_payload().as_string_view(); }
};

typedef Value ErrorMessage;
typedef int8_t ErrNo;

struct GetOptions : public ::z_get_options_t {
    GetOptions() : ::z_get_options_t(::z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) {
        target = v;
        return *this;
    }
    GetOptions& set_consolidation(z_consolidation_mode_t v) {
        consolidation.mode = v;
        return *this;
    }
};

struct PutOptions : public ::z_put_options_t {
    PutOptions() : ::z_put_options_t(::z_put_options_default()) {}
    PutOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
};

struct QueryReplyOptions : public ::z_query_reply_options_t {
    QueryReplyOptions() : ::z_query_reply_options_t(::z_query_reply_options_default()) {}
    QueryReplyOptions& set_encoding(Encoding e) {
        encoding = e;
        return *this;
    };
};

class Query : public ::z_query_t {
   public:
    Query() = delete;
    Query(::z_query_t query) : ::z_query_t(query) {}
    KeyExprView get_keyexpr() const { return KeyExprView(::z_query_keyexpr(this)); }
    Bytes get_parameters() const { return Bytes(::z_query_parameters(this)); }

    bool reply(KeyExprView key, const Bytes& payload, const QueryReplyOptions& options, ErrNo& error) const {
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const Bytes& payload, const QueryReplyOptions& options) const {
        ErrNo error;
        return reply_impl(key, payload, &options, error);
    }
    bool reply(KeyExprView key, const Bytes& payload, ErrNo& error) const {
        return reply_impl(key, payload, nullptr, error);
    }
    bool reply(KeyExprView key, const Bytes& payload) const {
        ErrNo error;
        return reply_impl(key, payload, nullptr, error);
    }

   private:
    bool reply_impl(KeyExprView key, const Bytes& payload, const QueryReplyOptions* options, ErrNo& error) const {
        error = ::z_query_reply(this, key, payload.start, payload.len, options);
        return error == 0;
    }
};

struct QueryableOptions : public ::z_queryable_options_t {
    QueryableOptions() : ::z_queryable_options_t(::z_queryable_options_default()) {}
    bool get_complete() const { return complete; }
    QueryableOptions& set_complete(bool v) {
        complete = v;
        return *this;
    }
};

}  // namespace zenoh
