//
// Copyright (c) 2025 ZettaScale Technology
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

#include <chrono>
#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

void test_all_tasks_auto_start() {
    std::cout << "Test tasks auto start: all\n";

    Session::SessionOptions opts;
    opts.start_background_tasks = true;
    auto session1 = Session::open(Config::create_default(), std::move(opts));

    assert(session1.is_read_task_running());
    assert(session1.is_lease_task_running());
    assert(session1.is_periodic_scheduler_task_running());

    opts.start_background_tasks = false;
    auto session2 = Session::open(Config::create_default(), std::move(opts));

    assert(!session2.is_read_task_running());
    assert(!session2.is_lease_task_running());
    assert(!session2.is_periodic_scheduler_task_running());
}

void test_individual_tasks_auto_start() {
    std::cout << "Test tasks auto start: individual\n";

    Session::SessionOptions opts;
    Session::SessionOptions::BackgroundTasksAutoStartOptions tasks;
    tasks.auto_start_read_task = true;
    tasks.auto_start_lease_task = false;
    tasks.auto_start_periodic_task = false;
    opts.start_background_tasks = tasks;

    auto session1 = Session::open(Config::create_default(), std::move(opts));

    assert(session1.is_read_task_running());
    assert(!session1.is_lease_task_running());
    assert(!session1.is_periodic_scheduler_task_running());

    tasks.auto_start_read_task = false;
    tasks.auto_start_lease_task = true;
    tasks.auto_start_periodic_task = false;
    opts.start_background_tasks = tasks;

    auto session2 = Session::open(Config::create_default(), std::move(opts));

    assert(!session2.is_read_task_running());
    assert(session2.is_lease_task_running());
    assert(!session2.is_periodic_scheduler_task_running());

    tasks.auto_start_read_task = false;
    tasks.auto_start_lease_task = false;
    tasks.auto_start_periodic_task = true;
    opts.start_background_tasks = tasks;

    auto session3 = Session::open(Config::create_default(), std::move(opts));

    assert(!session3.is_read_task_running());
    assert(!session3.is_lease_task_running());
    assert(session3.is_periodic_scheduler_task_running());
}

int main(int argc, char** argv) {
    test_all_tasks_auto_start();
    test_individual_tasks_auto_start();
};
