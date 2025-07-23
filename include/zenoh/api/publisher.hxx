//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#pragma once

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_PUBLICATION == 1

#include "../detail/closures_concrete.hxx"
#include "base.hxx"
#include "bytes.hxx"
#include "encoding.hxx"
#include "enums.hxx"
#include "interop.hxx"
#include "keyexpr.hxx"
#include "timestamp.hxx"
#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
#include "matching.hxx"
#endif
#if defined(Z_FEATURE_UNSTABLE_API)
#include "source_info.hxx"
#endif
#include <optional>

namespace zenoh {
class Session;

/// A Zenoh publisher. Constructed by ``Session::declare_publisher`` method.
class Publisher : public Owned<::z_owned_publisher_t> {
    Publisher(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @brief Options to be passed to ``Publisher::put`` operation.
    struct PutOptions {
        /// @name Fields

        /// @brief The encoding of the data to publish.
        std::optional<Encoding> encoding = {};
        /// @brief The timestamp of this message.
        std::optional<Timestamp> timestamp = {};
#if defined(Z_FEATURE_UNSTABLE_API)
        /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
        /// release.
        /// @brief The source info of this message.
        std::optional<SourceInfo> source_info = {};
#endif
        /// @brief The attachment to attach to the publication.
        std::optional<Bytes> attachment = {};

        /// @name Methods

        /// @brief Create default option settings.
        static PutOptions create_default() { return {}; }

       private:
        friend struct interop::detail::Converter;
        ::z_publisher_put_options_t to_c_opts() {
            ::z_publisher_put_options_t opts;
            z_publisher_put_options_default(&opts);
            opts.encoding = interop::as_moved_c_ptr(this->encoding);
#if defined(Z_FEATURE_UNSTABLE_API)
            opts.source_info = interop::as_moved_c_ptr(this->source_info);
#endif
            opts.attachment = interop::as_moved_c_ptr(this->attachment);
            opts.timestamp = interop::as_copyable_c_ptr(this->timestamp);
            return opts;
        }
    };

    /// @brief Options to be passed to ``Publisher::delete_resource`` operation.
    struct DeleteOptions {
        /// @name Fields

        /// @brief The timestamp of this message.
        std::optional<Timestamp> timestamp = {};

        /// @name Methods

        /// @brief Create default option settings.
        static DeleteOptions create_default() { return {}; }

       private:
        friend struct interop::detail::Converter;
        ::z_publisher_delete_options_t to_c_opts() {
            ::z_publisher_delete_options_t opts;
            z_publisher_delete_options_default(&opts);
            opts.timestamp = interop::as_copyable_c_ptr(this->timestamp);
            return opts;
        }
    };

    /// @name Methods

    /// @brief Publish a message on publisher key expression.
    /// @param payload data to publish.
    /// @param options optional parameters to pass to put operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void put(Bytes&& payload, PutOptions&& options = PutOptions::create_default(), ZResult* err = nullptr) const {
        auto payload_ptr = interop::as_moved_c_ptr(payload);
        ::z_publisher_put_options_t opts = interop::detail::Converter::to_c_opts(options);
        __ZENOH_RESULT_CHECK(::z_publisher_put(interop::as_loaned_c_ptr(*this), payload_ptr, &opts), err,
                             "Failed to perform put operation");
    }

    /// @brief Undeclare the resource associated with the publisher key expression.
    /// @param options optional parameters to pass to delete operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void delete_resource(DeleteOptions&& options = DeleteOptions::create_default(), ZResult* err = nullptr) const {
        ::z_publisher_delete_options_t opts = interop::detail::Converter::to_c_opts(options);
        __ZENOH_RESULT_CHECK(::z_publisher_delete(interop::as_loaned_c_ptr(*this), &opts), err,
                             "Failed to perform delete_resource operation");
    }

    /// @brief Get the key expression of the publisher.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::z_publisher_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Undeclares publisher.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::z_undeclare_publisher(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare publisher");
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the publisher.
    /// @return id of this publisher.
    EntityGlobalId get_id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(::z_publisher_id(interop::as_loaned_c_ptr(*this)));
    }
#endif

#if (defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1)
    /// @brief Construct matching listener, registering a callback for notifying subscribers matching with a given
    /// publisher.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the publisher
    /// changes (i.e. if last subscriber disconnects or when the first subscriber connects).
    /// @param on_drop the callable that will be called once matching listener is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``MatchingListener`` object.
    template <class C, class D>
    [[nodiscard]] MatchingListener<void> declare_matching_listener(C&& on_status_change, D&& on_drop,
                                                                   ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_status_change_call, detail::closures::_zenoh_on_drop,
                    closure);
        MatchingListener<void> m(zenoh::detail::null_object);
        ZResult res = ::z_publisher_declare_matching_listener(interop::as_loaned_c_ptr(*this),
                                                              interop::as_owned_c_ptr(m), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Matching Listener");
        return m;
    }

    /// @brief Construct matching listener, delivering notification on publisher status change through a streaming
    /// handler.
    /// @tparam Channel the type of channel used to create stream of data (see ``zenoh::channels::FifoChannel`` or
    /// ``zenoh::channels::RingChannel``).
    /// @param channel: an instance of channel.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``MatchingListener`` object.
    template <class Channel>
    [[nodiscard]] MatchingListener<typename Channel::template HandlerType<MatchingStatus>> declare_matching_listener(
        Channel channel, ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Query>();
        MatchingListener<void> m(zenoh::detail::null_object);
        ZResult res = ::z_publisher_declare_matching_listener(
            interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(m), ::z_move(cb_handler_pair.first));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Matching Listener");
        if (res != Z_OK) ::z_drop(interop::as_moved_c_ptr(cb_handler_pair.second));
        return MatchingListener<typename Channel::template HandlerType<MatchingStatus>>(
            std::move(m), std::move(cb_handler_pair.second));
    }

    /// @brief Declare matching listener, registering a callback for notifying subscribers matching with a given
    /// publisher. The callback will be run in the background until the corresponding publisher is destroyed.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the publisher
    /// changes (i.e. if last subscriber disconnects or when the first subscriber connects).
    /// @param on_drop the callable that will be called once publisher is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    template <class C, class D>
    void declare_background_matching_listener(C&& on_status_change, D&& on_drop, ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename detail::closures::Closure<Cval, Dval, void, const MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, detail::closures::_zenoh_on_status_change_call, detail::closures::_zenoh_on_drop,
                    closure);
        ZResult res =
            ::z_publisher_declare_background_matching_listener(interop::as_loaned_c_ptr(*this), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare background Matching Listener");
    }

    /// @brief Gets publisher matching status - i.e. if there are any subscribers matching its key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    MatchingStatus get_matching_status(ZResult* err = nullptr) const {
        ::z_matching_status_t m;
        ZResult res = ::z_publisher_get_matching_status(interop::as_loaned_c_ptr(*this), &m);
        __ZENOH_RESULT_CHECK(res, err, "Failed to get matching status");
        return {m.matching};
    }
#endif
};

}  // namespace zenoh
#endif
