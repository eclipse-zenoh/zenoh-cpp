#!/bin/sh
#
# Copyright (c) 2022 ZettaScale Technology
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
# which is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
#
# Contributors:
#   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
#

TESTBIN="$1"
TESTDIR=$(dirname "$0")
ZENOH_BRANCH="$2"

LOCATOR="tcp/127.0.0.1:7447"

# get vinary name without extension
TEST_NAME_WE=$(basename -- "$TESTBIN")
TEST_NAME_WE="${TEST_NAME_WE%.*}"

cd "$TESTDIR"|| exit

echo "------------------ Running test $TESTBIN -------------------"

cleanup() {
    stop_router
}

trap cleanup EXIT INT TERM

stop_router() {
    if [ -z "${ZPID:-}" ]; then
        return
    fi

    kill -TERM "$ZPID" 2>/dev/null || true

    attempts=0
    while kill -0 "$ZPID" 2>/dev/null && [ "$attempts" -lt 20 ]; do
        sleep 0.1
        attempts=$((attempts + 1))
    done

    if kill -0 "$ZPID" 2>/dev/null; then
        kill -KILL "$ZPID" 2>/dev/null || true
    fi

    wait "$ZPID" 2>/dev/null || true
    ZPID=
}

sleep 5

if [ ! -f zenohd ]; then
    git clone https://github.com/eclipse-zenoh/zenoh.git zenoh-git
    cd zenoh-git || exit
    git switch "$ZENOH_BRANCH"
    rustup show
    cargo build --lib --bin zenohd
    cp ./target/debug/zenohd "$TESTDIR"/
    cd "$TESTDIR"|| exit
fi

chmod +x zenohd

sleep 1

echo "> Running zenohd ... $LOCATOR"
RUST_LOG=debug ./zenohd --plugin-search-dir "$TESTDIR/zenoh-git/target/debug" -l "$LOCATOR" > zenohd."$TEST_NAME_WE".log 2>&1 &
ZPID=$!

sleep 5

echo "> Running $TESTBIN ..."
if command -v stdbuf >/dev/null 2>&1; then
    stdbuf -oL -eL "$TESTBIN" > client."$TEST_NAME_WE".log 2>&1
else
    "$TESTBIN" > client."$TEST_NAME_WE".log 2>&1
fi
RETCODE=$?

echo "> Logs of $TESTBIN ..."
cat client."$TEST_NAME_WE".log

echo "> Stopping zenohd ..."
stop_router

sleep 1

echo "> Logs of zenohd ..."
cat zenohd."$TEST_NAME_WE".log

[ "$RETCODE" -lt 0 ] && exit "$RETCODE"

echo "> Done ($RETCODE)."
exit "$RETCODE"
