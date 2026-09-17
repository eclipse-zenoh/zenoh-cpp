//
// Copyright (c) 2017, 2022 ZettaScale Technology.
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh team, <zenoh@zettascale.tech>
//

#pragma once

// zenoh-c's non-blocking wasm32 API (zc_runtime_t, zc_open_*) only exists in builds targeting
// wasm32 without atomics -- see zenoh-c's src/wasm_runtime.rs. zenoh-pico has its own, unrelated
// wasm32 story (a real single-threaded C library, not a Rust runtime to drive), so this is
// zenoh-c-only.
#if defined(ZENOHCXX_ZENOHC) && defined(__wasm32__)

#include <optional>
#include <utility>

#include "session.hxx"

namespace zenoh {

/// @brief A single-threaded runtime used to drive zenoh's async internals on targets with no
/// real threads (e.g. wasm32-unknown-emscripten without `-pthread`).
///
/// zenoh-c's normal, synchronous API (``z_open()`` and friends) relies on ``block_in_place``,
/// which panics on this target. ``Runtime`` and ``SessionOpenTask`` are the non-blocking
/// alternative for opening a session; everything declared on the resulting ``Session``
/// (subscribers, publishers, queryables, ...) works normally afterwards, as long as
/// ``pump_once()`` keeps getting called -- no other zenoh-c call on this target performs real
/// I/O by itself.
class Runtime {
   public:
    /// @brief Creates a new runtime.
    /// @param err if not null, the result code will be written to this location, otherwise
    /// ZException will be thrown in case of error.
    explicit Runtime(ZResult* err = nullptr) : _rt(::zc_runtime_new()) {
        if (_rt == nullptr) {
            __ZENOH_RESULT_CHECK(Z_EUNAVAILABLE, err, "Failed to create Runtime");
        }
    }

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Runtime(Runtime&& other) noexcept : _rt(std::exchange(other._rt, nullptr)) {}
    Runtime& operator=(Runtime&& other) noexcept {
        if (this != &other) {
            if (_rt != nullptr) ::zc_runtime_free(_rt);
            _rt = std::exchange(other._rt, nullptr);
        }
        return *this;
    }

    ~Runtime() {
        if (_rt != nullptr) ::zc_runtime_free(_rt);
    }

    /// @brief Drives the runtime forward by one non-blocking step: runs any zenoh-internal
    /// tasks that are ready (delivering a subscriber callback, flushing a batch to the wire,
    /// ...), then returns immediately regardless of what became ready.
    ///
    /// Call this repeatedly from an external loop (e.g. via ``emscripten_set_main_loop_arg``);
    /// it never blocks and never sleeps.
    void pump_once() const { ::zc_runtime_pump_once(_rt); }

   private:
    friend class SessionOpenTask;
    ::zc_runtime_t* loan() const { return _rt; }

    ::zc_runtime_t* _rt;
};

/// @brief A pending, resumable ``zenoh::open()`` call.
///
/// Poll it with ``poll()``, alongside ``Runtime::pump_once()``, until it reports the session is
/// ready (or has failed).
class SessionOpenTask {
   public:
    /// @brief Starts opening a session without blocking. ``config`` is consumed either way.
    /// @param config Zenoh session ``Config``.
    /// @param err if not null, the result code will be written to this location, otherwise
    /// ZException will be thrown in case of error (e.g. if ``config`` was already moved-from).
    explicit SessionOpenTask(Config&& config, ZResult* err = nullptr)
        : _task(::zc_open_start(interop::as_moved_c_ptr(config))) {
        if (_task == nullptr) {
            __ZENOH_RESULT_CHECK(Z_EINVAL, err, "Failed to start opening session");
        }
    }

    SessionOpenTask(const SessionOpenTask&) = delete;
    SessionOpenTask& operator=(const SessionOpenTask&) = delete;

    SessionOpenTask(SessionOpenTask&& other) noexcept : _task(std::exchange(other._task, nullptr)) {}
    SessionOpenTask& operator=(SessionOpenTask&& other) noexcept {
        if (this != &other) {
            if (_task != nullptr) ::zc_open_task_free(_task);
            _task = std::exchange(other._task, nullptr);
        }
        return *this;
    }

    ~SessionOpenTask() {
        if (_task != nullptr) ::zc_open_task_free(_task);
    }

    /// @brief Polls this task. Does not itself drive I/O -- call ``Runtime::pump_once()`` in the
    /// same loop so the underlying connection can actually progress.
    /// @param rt the ``Runtime`` this task was started against.
    /// @param err if not null, the result code will be written to this location on failure,
    /// otherwise ZException will be thrown in case of error.
    /// @return ``std::nullopt`` if the session is not open yet (call again after the next
    /// ``Runtime::pump_once()``), otherwise the opened ``Session``.
    std::optional<Session> poll(const Runtime& rt, ZResult* err = nullptr) {
        ::z_owned_session_t raw_session;
        int8_t code = ::zc_open_poll(rt.loan(), _task, &raw_session);
        if (code == -1) {
            return std::nullopt;
        }
        if (code == 0) {
            return std::optional<Session>(std::move(interop::as_owned_cpp_ref<Session>(&raw_session)));
        }
        __ZENOH_RESULT_CHECK(code, err, "Failed to open session");
        return std::nullopt;
    }

   private:
    ::zc_open_task_t* _task;
};

}  // namespace zenoh

#endif  // defined(ZENOHCXX_ZENOHC) && defined(__wasm32__)
