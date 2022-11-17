#pragma once

#include "zenoh.h"
#include <string_view>

namespace zenoh
{

struct Bytes : public ::z_bytes_t {
    // operator std::string_view() const { return std::string_view(to_const_char_ptr(start),len); }
    // static const char* to_const_char_ptr(const uint8_t* buf) { return reinterpret_cast<const char*>(buf); };
    std::string_view as_string_view() const { return std::string_view(reinterpret_cast<const char*>(start),len); }
};

struct KeyExprView : public ::z_keyexpr_t {
    KeyExprView(const char* name) : ::z_keyexpr_t(z_keyexpr(name)) {}
    KeyExprView(const z_keyexpr_t& v) : ::z_keyexpr_t(v) {}
    bool check() const { return z_keyexpr_is_initialized(this); }
    Bytes as_bytes() const { return Bytes { ::z_keyexpr_as_bytes(*this) }; }
};

struct GetOptions : public ::z_get_options_t
{
    GetOptions() : ::z_get_options_t(::z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) { target = v; return *this; }
    GetOptions& set_consolidation(z_consolidation_mode_t v) { consolidation.mode = v; return *this; }
};

struct Sample : public ::z_sample_t {
    const KeyExprView& get_keyexpr() const { return static_cast<const KeyExprView&>(keyexpr); }
    const Bytes& get_payload() const { return static_cast<const Bytes&>(payload); }
};


}
