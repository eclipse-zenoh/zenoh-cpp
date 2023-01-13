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

#include "zenohcpp.h"

using namespace zenoh;

#undef NDEBUG
#include <assert.h>

void encoding() {
    Encoding def1;
    Encoding def2;
    assert(def1 == def2);

    Encoding foo(EncodingPrefix::Z_ENCODING_PREFIX_APP_FLOAT, "foo");
    Encoding bar(EncodingPrefix::Z_ENCODING_PREFIX_APP_INTEGER, "bar");
    assert(foo != bar);
    foo = bar;
    assert(foo == bar);

    Encoding encoding;
    const char* foobar = "foobar";
    encoding.set_prefix(EncodingPrefix::Z_ENCODING_PREFIX_APP_CUSTOM).set_suffix(std::string_view(foobar, 3));
    assert(encoding.get_prefix() == EncodingPrefix::Z_ENCODING_PREFIX_APP_CUSTOM);
    assert(encoding.get_suffix() ==
           "foo");  // check that it can work witn non null-terminated strings. Though it doesn't
                    // guarantee that such strinngs are correctly processed everywhere in zehon-c
}

void value() {
    const char* foobar = "foobar";
    Value foo("foo");
    Value bar(std::string_view(foobar + 3), Encoding(EncodingPrefix::Z_ENCODING_PREFIX_APP_JSON));
    assert(foo != bar);
    foo.set_encoding(EncodingPrefix::Z_ENCODING_PREFIX_APP_JSON).set_payload("bar");
    assert(foo == bar);
    assert(bar.get_encoding() == Encoding(EncodingPrefix::Z_ENCODING_PREFIX_APP_JSON));
    assert(bar.get_payload() == "bar");
}

void get_options() {
    GetOptions opts;
    opts.set_consolidation(QueryConsolidation())
        .set_consolidation(ConsolidationMode::Z_CONSOLIDATION_MODE_AUTO)
        .set_target(Z_QUERY_TARGET_ALL)
        .set_with_value("TEST");

    GetOptions opts2 = opts;
    assert(opts2 == opts);
    assert(opts.get_consolidation() == QueryConsolidation(ConsolidationMode::Z_CONSOLIDATION_MODE_AUTO));
    assert(opts.get_target() == QueryTarget::Z_QUERY_TARGET_ALL);
    assert(opts.get_with_value() == Value("TEST"));

    opts2.set_consolidation(Z_CONSOLIDATION_MODE_LATEST);
    assert(opts2 != opts);
}

void put_options() {
    PutOptions opts;
    opts.set_encoding(EncodingPrefix::Z_ENCODING_PREFIX_TEXT_PLAIN)
        .set_congestion_control(CongestionControl::Z_CONGESTION_CONTROL_BLOCK)
        .set_priority(Priority::Z_PRIORITY_DATA_HIGH);

    PutOptions opts2 = opts;
    assert(opts2 == opts);
    assert(opts.get_encoding() == Encoding(EncodingPrefix::Z_ENCODING_PREFIX_TEXT_PLAIN));
    assert(opts.get_congestion_control() == CongestionControl::Z_CONGESTION_CONTROL_BLOCK);
    assert(opts.get_priority() == Priority::Z_PRIORITY_DATA_HIGH);

    opts2.set_priority(Priority::Z_PRIORITY_DATA_LOW);
    assert(opts2 != opts);
}

int main(int argc, char** argv) {
    encoding();
    value();
    get_options();
    put_options();
};
