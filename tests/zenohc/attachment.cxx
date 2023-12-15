//
// Copyright (c) 2023 ZettaScale Technology
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

#include <map>
#include <string>

#include "zenohc.hxx"

using namespace zenohc;

#undef NDEBUG
#include <assert.h>

void writting_through_map_read_by_get() {
    // Writing
    std::map<std::string, std::string> amap;
    amap.insert(std::make_pair("k1", "v1"));
    amap.insert(std::make_pair("k2", "v2"));
    AttachmentView attachment(amap);

    // Elements check
    assert(attachment.get("k1").check());
    assert(attachment.get("k2").check());
    assert(!attachment.get("k_non").check());

    assert(attachment.get("k1").as_string_view() == "v1");
    assert(attachment.get("k2").as_string_view() == "v2");
}

bool _attachment_reader(const BytesView& key, const BytesView& value,
                        const std::vector<std::pair<std::string, std::string>>& test_data) {
    for (const auto& it : test_data) {
        if (it.first == key.as_string_view()) {
            assert(it.second == value.as_string_view());
            return true;
        }
    }
    assert(!"Unexpected");
}

void writting_through_map_read_by_iter_func() {
    std::vector<std::pair<std::string, std::string>> test_data;
    test_data.push_back(std::make_pair("k1", "v1"));
    test_data.push_back(std::make_pair("k2", "v2"));
    test_data.push_back(std::make_pair("k3", "v3"));

    // Writing
    std::map<std::string, std::string> amap;
    AttachmentView attachment(amap);

    // amap passed by reference we can change it after
    for (auto it : test_data) {
        amap.insert(std::make_pair(it.first, it.second));
    }

    // Elements check as function
    using namespace std::placeholders;
    attachment.iterate(std::bind(_attachment_reader, _1, _2, test_data));
}

void writting_through_map_read_by_iter_lambda() {
    std::vector<std::pair<std::string, std::string>> test_data;
    test_data.push_back(std::make_pair("k1", "v1"));
    test_data.push_back(std::make_pair("k2", "v2"));
    test_data.push_back(std::make_pair("k3", "v3"));

    // Writing
    std::map<std::string, std::string> amap;
    AttachmentView attachment(amap);

    // amap passed by reference we can change it after
    for (auto it : test_data) {
        amap.insert(std::make_pair(it.first, it.second));
    }

    // Elements check as lambda
    using namespace std::placeholders;
    attachment.iterate([&test_data](const BytesView& key, const BytesView& value) {
        for (const auto& it : test_data) {
            if (it.first == key.as_string_view()) {
                assert(it.second == value.as_string_view());
                return true;
            }
        }
        assert(!"Unexpected");
    });
}

void writting_no_map_read_by_get() {
    std::vector<std::pair<std::string, std::string>> test_data;
    test_data.push_back(std::make_pair("k1", "v1"));
    test_data.push_back(std::make_pair("k2", "v2"));
    test_data.push_back(std::make_pair("k3", "v3"));

    AttachmentView::IterDriver driver([&test_data](const AttachmentView::IterBody& body) -> bool {
        for (const auto& it : test_data) {
            if (body(BytesView(it.first), BytesView(it.second))) {
                return true;
            }
        }
        return false;
    });

    AttachmentView attachment(driver);

    // Elements check
    for (auto it : test_data) {
        assert(attachment.get(it.first).check());
        assert(attachment.get(it.first).as_string_view() == it.second);
    }
    assert(!attachment.get("k_non").check());
}

int main(int argc, char** argv) {
    init_logger();
    writting_through_map_read_by_get();
    writting_through_map_read_by_iter_func();
    writting_through_map_read_by_iter_lambda();
    writting_no_map_read_by_get();
}
