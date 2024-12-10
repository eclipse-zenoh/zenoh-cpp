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
#include <stdio.h>
#include <string.h>

#include <vector>

#include "../getargs.hxx"
#include "zenoh.hxx"
using namespace zenoh;

int _main(int argc, char **argv) {
    const char *keyexpr = "test/thr";
    auto &&[config, args] =
        ConfigCliArgParser(argc, argv)
            .positional("PAYLOAD_SIZE", "Size of the payload to publish (number)")
            .named_value({"s", "shared-memory"}, "SHARED_MEMORY_SIZE", "Shared memory size in MBytes", "32")
            .run();

    size_t len = std::atoi(args.positional(0).data());
    size_t shared_memory_size_mb = std::atoi(args.value("s").data());

    std::cout << "Opening session...\n";
    auto session = Session::open(std::move(config));

    std::cout << "Declaring Publisher on " << keyexpr << "...\n";

    Session::PublisherOptions options;
    options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
    auto pub = session.declare_publisher(keyexpr, std::move(options));

    std::cout << "Preparing SHM Provider...\n";
    PosixShmProvider provider(MemoryLayout(shared_memory_size_mb * 1024 * 1024, AllocAlignment({2})));

    std::cout << "Allocating SHM buffer...\n";
    auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
    ZShmMut &&buf_mut = std::get<ZShmMut>(std::move(alloc_result));
    ZShm buf(std::move(buf_mut));

    printf("Press CTRL-C to quit...\n");
    while (1) pub.put(ZShm(buf));
}

int main(int argc, char **argv) {
    try {
#ifdef ZENOHCXX_ZENOHC
        init_log_from_env_or("error");
#endif
        _main(argc, argv);
    } catch (ZException e) {
        std::cout << "Received an error :" << e.what() << "\n";
    }
}
