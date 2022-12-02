//
// Copyright (c) 2022 ZettaScale Technology
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

#include "zenohcpp_base.h"
#include "zenohcpp_structs.h"

namespace zenoh {

class ClosureReplyChannelSend : public ClosureReply {
   public:
    using ClosureReply::ClosureReply;
};

class ClosureReplyChannelRecv : public Owned<::z_owned_reply_channel_closure_t> {
   public:
    using Owned::Owned;
};

std::pair<ClosureReplyChannelSend, ClosureReplyChannelRecv> reply_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

std::pair<ClosureReplyChannelSend, ClosureReplyChannelRecv> reply_non_blocking_fifo_new(uintptr_t bound) {
    auto channel = ::zc_reply_non_blocking_fifo_new(bound);
    return {std::move(channel.send), std::move(channel.recv)};
}

}  // namespace zenoh