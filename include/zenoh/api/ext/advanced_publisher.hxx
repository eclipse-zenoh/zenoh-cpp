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

#if defined(ZENOHCXX_ZENOHC) && defined(Z_FEATURE_UNSTABLE_API)
#include <optional>

#include "../../detail/closures_concrete.hxx"
#include "../base.hxx"
#include "../bytes.hxx"
#include "../encoding.hxx"
#include "../enums.hxx"
#include "../interop.hxx"
#include "../keyexpr.hxx"
#include "../publisher.hxx"
#include "../sample.hxx"
#include "../timestamp.hxx"
#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1
#include "../matching.hxx"
#endif
#if defined(Z_FEATURE_UNSTABLE_API)
#include "../source_info.hxx"
#endif

namespace zenoh::ext {

/// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future release.
/// @brief A Zenoh advanced publisher.
///
/// In addition to publishing the data,
/// it also maintains the storage, allowing matching subscribers to retrive missed samples.
/// @note Zenoh-c only
class AdvancedPublisher : public Owned<::ze_owned_advanced_publisher_t> {
    AdvancedPublisher(zenoh::detail::null_object_t) : Owned(nullptr){};
    friend struct interop::detail::Converter;

   public:
    /// @name Methods

    /// @brief Get the key expression of the advanced publisher.
    const KeyExpr& get_keyexpr() const {
        return interop::as_owned_cpp_ref<KeyExpr>(::ze_advanced_publisher_keyexpr(interop::as_loaned_c_ptr(*this)));
    }

    /// @brief Undeclare advanced publisher.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void undeclare(ZResult* err = nullptr) && {
        __ZENOH_RESULT_CHECK(::ze_undeclare_advanced_publisher(interop::as_moved_c_ptr(*this)), err,
                             "Failed to undeclare Advanced Publisher");
    }

#if defined(Z_FEATURE_UNSTABLE_API)
    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Get the id of the advanced publisher.
    /// @return id of this advancedc publisher.
    EntityGlobalId get_id() const {
        return interop::into_copyable_cpp_obj<EntityGlobalId>(
            ::ze_advanced_publisher_id(interop::as_loaned_c_ptr(*this)));
    }
#endif

    /// @brief Options to be passed to ``AdvancedPublisher::put`` operation.
    struct PutOptions {
        /// @name Fields

        /// Base put options.
        zenoh::Publisher::PutOptions put_options = {};

        /// @name Methods

        /// @brief Create default option settings.
        static PutOptions create_default() { return {}; }

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_advanced_publisher_put_options_t to_c_opts() {
            ::ze_advanced_publisher_put_options_t opts;
            ze_advanced_publisher_put_options_default(&opts);
            opts.put_options = zenoh::interop::detail::Converter::to_c_opts(this->put_options);
            return opts;
        }
    };

    /// @brief Options to be passed to ``AdvancedPublisher::delete_resource`` operation.
    struct DeleteOptions {
        /// @name Fields

        /// Base delete options.
        zenoh::Publisher::DeleteOptions delete_options = {};

        /// @name Methods

        /// @brief Create default option settings.
        static DeleteOptions create_default() { return {}; }

       private:
        friend struct zenoh::interop::detail::Converter;
        ::ze_advanced_publisher_delete_options_t to_c_opts() {
            ::ze_advanced_publisher_delete_options_t opts;
            ze_advanced_publisher_delete_options_default(&opts);
            opts.delete_options = zenoh::interop::detail::Converter::to_c_opts(this->delete_options);
            return opts;
        }
    };

    /// @name Methods

    /// @brief Publish a message on advanced publisher key expression.
    /// @param payload data to publish.
    /// @param options optional parameters to pass to put operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void put(Bytes&& payload, PutOptions&& options = PutOptions::create_default(), zenoh::ZResult* err = nullptr) {
        auto payload_ptr = interop::as_moved_c_ptr(payload);
        ::ze_advanced_publisher_put_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        __ZENOH_RESULT_CHECK(::ze_advanced_publisher_put(interop::as_loaned_c_ptr(*this), payload_ptr, &opts), err,
                             "Failed to perform put operation");
    }

    /// @brief Undeclare the resource associated with the advanced publisher key expression.
    /// @param options optional parameters to pass to delete operation.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    void delete_resource(DeleteOptions&& options = DeleteOptions::create_default(),
                         zenoh::ZResult* err = nullptr) const {
        ::ze_advanced_publisher_delete_options_t opts = zenoh::interop::detail::Converter::to_c_opts(options);
        __ZENOH_RESULT_CHECK(::ze_advanced_publisher_delete(interop::as_loaned_c_ptr(*this), &opts), err,
                             "Failed to perform delete_resource operation");
    }

#if defined(ZENOHCXX_ZENOHC) || Z_FEATURE_MATCHING == 1
    /// @brief Construct matching listener, registering a callback for notifying subscribers matching with a given
    /// advanced publisher.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the publisher
    /// changes (i.e. if last subscriber disconnects or when the first subscriber connects).
    /// @param on_drop the callable that will be called once matching listener is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @return a ``MatchingListener`` object.
    template <class C, class D>
    [[nodiscard]] MatchingListener<void> declare_matching_listener(C&& on_status_change, D&& on_drop,
                                                                   zenoh::ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const zenoh::MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const zenoh::MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_status_change_call,
                    zenoh::detail::closures::_zenoh_on_drop, closure);
        auto m = zenoh::interop::detail::null<MatchingListener<void>>();
        zenoh::ZResult res = ::ze_advanced_publisher_declare_matching_listener(
            zenoh::interop::as_loaned_c_ptr(*this), zenoh::interop::as_owned_c_ptr(m), ::z_move(c_closure));
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
        Channel channel, zenoh::ZResult* err = nullptr) const {
        auto cb_handler_pair = channel.template into_cb_handler_pair<Query>();
        auto m = zenoh::interop::detail::null<MatchingListener<void>>();
        zenoh::ZResult res = ::z_publisher_declare_matching_listener(
            interop::as_loaned_c_ptr(*this), interop::as_owned_c_ptr(m), ::z_move(cb_handler_pair.first));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare Matching Listener");
        if (res != Z_OK) ::z_drop(zenoh::interop::as_moved_c_ptr(cb_handler_pair.second));
        return MatchingListener<typename Channel::template HandlerType<MatchingStatus>>(
            std::move(m), std::move(cb_handler_pair.second));
    }

    /// @warning This API has been marked as unstable: it works as advertised, but it may be changed in a future
    /// release.
    /// @brief Declare matching listener, registering a callback for notifying subscribers matching with a given
    /// advanced publisher. The callback will be run in the background until the corresponding publisher is destroyed.
    ///
    /// @param on_status_change: the callable that will be called every time the matching status of the publisher
    /// changes (i.e. if last subscriber disconnects or when the first subscriber connects).
    /// @param on_drop the callable that will be called once publisher is destroyed or undeclared.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    /// @note Zenoh-c only.
    template <class C, class D>
    void declare_background_matching_listener(C&& on_status_change, D&& on_drop, zenoh::ZResult* err = nullptr) const {
        static_assert(std::is_invocable_r<void, C, const zenoh::MatchingStatus&>::value,
                      "on_status_change should be callable with the following signature: void on_status_change(const "
                      "zenoh::MatchingStatus& status)");
        static_assert(std::is_invocable_r<void, D>::value,
                      "on_drop should be callable with the following signature: void on_drop()");
        ::z_owned_closure_matching_status_t c_closure;
        using Cval = std::remove_reference_t<C>;
        using Dval = std::remove_reference_t<D>;
        using ClosureType = typename zenoh::detail::closures::Closure<Cval, Dval, void, const zenoh::MatchingStatus&>;
        auto closure = ClosureType::into_context(std::forward<C>(on_status_change), std::forward<D>(on_drop));
        ::z_closure(&c_closure, zenoh::detail::closures::_zenoh_on_status_change_call,
                    zenoh::detail::closures::_zenoh_on_drop, closure);
        zenoh::ZResult res = ::ze_advanced_publisher_declare_background_matching_listener(
            zenoh::interop::as_loaned_c_ptr(*this), ::z_move(c_closure));
        __ZENOH_RESULT_CHECK(res, err, "Failed to declare background Matching Listener");
    }

    /// @brief Gets advanced publisher matching status - i.e. if there are any subscribers matching its key expression.
    /// @param err if not null, the result code will be written to this location, otherwise ZException exception will be
    /// thrown in case of error.
    MatchingStatus get_matching_status(zenoh::ZResult* err = nullptr) const {
        ::z_matching_status_t m;
        zenoh::ZResult res = ::ze_advanced_publisher_get_matching_status(zenoh::interop::as_loaned_c_ptr(*this), &m);
        __ZENOH_RESULT_CHECK(res, err, "Failed to get matching status");
        return {m.matching};
    }
#endif
};

}  // namespace zenoh::ext
#endif
