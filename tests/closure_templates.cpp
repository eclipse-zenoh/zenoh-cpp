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
//
#include <assert.h>

#include "zenohcpp.h"

using namespace zenoh;

void on_reply_lv(Reply){};
void on_reply_rv(Reply&){};
void on_reply_rlv(Reply&&){};
void on_reply_opt(std::optional<Reply>){};

int main(int argc, char** argv) {
    ClosureReply closure_reply_lv([](Reply) {});
    ClosureReply closure_reply_rv([](Reply&) {});
    ClosureReply closure_reply_rlv([](Reply&&) {});
    ClosureReply closure_reply_opt([](std::optional<Reply>) {});
    ClosureReply closure_reply_flv(on_reply_lv);
    // ClosureReply closure_reply_frv(on_reply_rv);
    // ClosureReply closure_reply_frlv(on_reply_rlv);
    // ClosureReply closure_reply_fopt(on_reply_opt);
}
