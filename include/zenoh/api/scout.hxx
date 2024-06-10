

#pragma once

#include "base.hxx"
#include "../detail/interop.hxx"
#include "../detail/closures_concrete.hxx"

#include "enums.hxx"
#include "config.hxx"

namespace zenoh {

/// @brief Options to be passed to ``scout()`` operation
struct ScoutOptions {
    /// @brief The maximum duration in ms the scouting can take.
    size_t timeout_ms = 1000;
    /// @brief Type of entities to scout for.
    WhatAmI what = WhatAmI::Z_WHATAMI_ROUTER_PEER;

    /// @brief Returns default option settings
    static ScoutOptions create_default() { return {}; }
};

/// @brief Scout for zenoh entities in the network
/// @param config ``ScoutingConfig`` to use
/// @param on_hello The callback to process each received ``Hello``message
/// @param on_drop The callback that will be called once all hello ``Hello``messages are received
template<class C, class D>
void scout(Config&& config, C&& on_hello, D&& on_drop, ScoutOptions&& options = ScoutOptions::create_default(), ZError* err = nullptr) {
    static_assert(
        std::is_invocable_r<void, C, const Hello&>::value,
        "on_hello should be callable with the following signature: void on_hello(const zenoh::Hello& hello)"
    );
    static_assert(
        std::is_invocable_r<void, D>::value,
        "on_drop should be callable with the following signature: void on_drop()"
    );
    ::z_owned_closure_hello_t c_closure;
    using ClosureType = typename detail::closures::Closure<C, D, void, const Hello&>;
    auto closure = ClosureType::into_context(std::forward<C>(on_hello), std::forward<D>(on_drop));
    ::z_closure(&c_closure, detail::closures::_zenoh_on_hello_call, detail::closures::_zenoh_on_drop, closure);
    ::z_scout_options_t opts;
    opts.zc_timeout_ms = options.timeout_ms;
    opts.zc_what = options.what;

    __ZENOH_ERROR_CHECK(
        ::z_scout(detail::as_owned_c_ptr(config), ::z_move(c_closure), &opts),
        err,
        "Failed to perform scout operation"
    );
}

}