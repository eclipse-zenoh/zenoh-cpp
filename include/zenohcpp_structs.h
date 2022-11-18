#pragma once

#include "zenoh.h"
#include <string_view>
#include "string.h"

namespace zenoh
{

struct Bytes : public ::z_bytes_t {
    Bytes() : ::z_bytes_t({}) {}
    Bytes(::z_bytes_t v) : ::z_bytes_t(v) {}
    Bytes(const char* s) : ::z_bytes_t({
        start: reinterpret_cast<const uint8_t*>(s),
        len: strlen(s)
    }) {}
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start),len); }
};

struct KeyExprView : public ::z_keyexpr_t {
    KeyExprView(const char* name) : ::z_keyexpr_t(z_keyexpr(name)) {}
    KeyExprView(const z_keyexpr_t& v) : ::z_keyexpr_t(v) {}
    bool check() const { return z_keyexpr_is_initialized(this); }
    Bytes as_bytes() const { return Bytes { ::z_keyexpr_as_bytes(*this) }; }
    std::string_view as_string_view() const { return as_bytes().as_string_view(); }
};

typedef ::z_encoding_prefix_t EncodingPrefix;

struct Encoding : public ::z_encoding_t {
    Encoding(EncodingPrefix _prefix) : ::z_encoding_t(::z_encoding(_prefix, nullptr)) {}
    Encoding(EncodingPrefix _prefix, const char* _suffix) : ::z_encoding_t(::z_encoding(_prefix, _suffix)) {}
};

struct Sample : public ::z_sample_t {
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
};

struct Value : public ::z_value_t {
    const Bytes& get_payload() const {  return static_cast<const Bytes&>(payload); }
    const Encoding& get_encoding() const { return static_cast<const Encoding&>(encoding); }
    std::string_view as_string_view() const { return get_payload().as_string_view(); }
};

typedef Value Error;

struct GetOptions : public ::z_get_options_t
{
    GetOptions() : ::z_get_options_t(::z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) { target = v; return *this; }
    GetOptions& set_consolidation(z_consolidation_mode_t v) { consolidation.mode = v; return *this; }
};

struct PutOptions : public ::z_put_options_t {
    PutOptions() : ::z_put_options_t(::z_put_options_default()) {}
    PutOptions& set_encoding(Encoding e) { encoding = e; return *this; };
};


}
