

#pragma once

#include "../detail/closures_concrete.hxx"
#include "base.hxx"
#include "config.hxx"
#include "enums.hxx"
#include "interop.hxx"

namespace zenoh {

/// @brief Options to be passed to ``scout`` operation
struct ScoutOptions {
    /// @name Fields

    /// @brief The maximum duration in ms the scouting can take.
    size_t timeout_ms = 1000;
    /// @brief Type of entities to scout for.
    What what = What::Z_WHAT_ROUTER_PEER;

    /// @name Methods

    /// @brief Create default option settings.
    static ScoutOptions create_default() { return {}; }
};

/// @brief Scout for zenoh entities in the network.
/// @param config ``Config`` to use for scouting.
/// @param on_hello the callable to process each received ``Hello``message.
/// @param on_drop the callable that will be called once all ``Hello`` messages are received.
template <class C, class D>
void scout(Config&& config, C&& on_hello, D&& on_drop, ScoutOptions&& options = ScoutOptions::create_default(),
           ZResult* err = nullptr) {
    static_assert(std::is_invocable_r<void, C, const Hello&>::value,
                  "on_hello should be callable with the following signature: void on_hello(zenoh::Hello& hello)");
    static_assert(std::is_invocable_r<void, D>::value,
                  "on_drop should be callable with the following signature: void on_drop()");
    ::z_owned_closure_hello_t c_closure;
    using Cval = std::remove_reference_t<C>;
    using Dval = std::remove_reference_t<D>;
    using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const Hello&>;
    auto closure = ClosureType::into_context(std::forward<C>(on_hello), std::forward<D>(on_drop));
    ::z_closure(&c_closure, detail::closures::_zenoh_on_hello_call, detail::closures::_zenoh_on_drop, closure);
    ::z_scout_options_t opts;
    opts.timeout_ms = options.timeout_ms;
    opts.what = options.what;

    __ZENOH_RESULT_CHECK(::z_scout(interop::as_moved_c_ptr(config), ::z_move(c_closure), &opts), err,
                         "Failed to perform scout operation");
}

}  // namespace zenoh