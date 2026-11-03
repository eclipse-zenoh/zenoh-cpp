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
ZENOH_BRANCH="$2"
TESTDIR=$(cd "$(dirname "$0")" && pwd) || exit 1
LOCATOR="tcp/127.0.0.1:7447"
TEST_NAME_WE=$(basename "$TESTBIN")
TEST_NAME_WE="${TEST_NAME_WE%.*}"
cd "$TESTDIR" || exit 1

CLIENT_LOG="$TESTDIR/client.$TEST_NAME_WE.log"
ROUTER_LOG="$TESTDIR/zenohd.$TEST_NAME_WE.log"
: > "$CLIENT_LOG"
: > "$ROUTER_LOG"

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

cleanup() {
    result=$?
    trap - EXIT INT TERM
    stop_router
    echo "> Client log: $CLIENT_LOG"
    cat "$CLIENT_LOG"
    echo "> Router log: $ROUTER_LOG"
    cat "$ROUTER_LOG"
    exit "$result"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

ROUTER=./zenohd
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) ROUTER=./zenohd.exe ;;
esac
if [ ! -f "$ROUTER" ]; then
    git clone https://github.com/eclipse-zenoh/zenoh.git zenoh-git || exit 1
    cd zenoh-git || exit 1
    git switch "$ZENOH_BRANCH" || exit 1
    rustup show
    cargo build --lib --bin zenohd || exit 1
    cp "./target/debug/${ROUTER#./}" "$TESTDIR/" || exit 1
    cd "$TESTDIR" || exit 1
fi
chmod +x "$ROUTER" || exit 1
echo "> Running zenohd ... $LOCATOR"
RUST_LOG=debug "$ROUTER" --plugin-search-dir "$TESTDIR/zenoh-git/target/debug" -l "$LOCATOR" > "$ROUTER_LOG" 2>&1 &
ZPID=$!

# Wait for this router to advertise its listener, not just for a fixed delay.
attempts=0
while ! grep -Fq "Zenoh can be reached at: $LOCATOR" "$ROUTER_LOG"; do
    if ! kill -0 "$ZPID" 2>/dev/null; then
        wait "$ZPID"
        router_status=$?
        ZPID=
        echo "Router exited before readiness: status=$router_status" >&2
        exit 1
    fi
    if [ "$attempts" -ge 30 ]; then
        echo "Router readiness timed out after 30 seconds" >&2
        exit 1
    fi
    sleep 1
    attempts=$((attempts + 1))
done

echo "> Running $TESTBIN ..."
if command -v stdbuf >/dev/null 2>&1; then
    stdbuf -oL -eL "$TESTBIN" > "$CLIENT_LOG" 2>&1
else
    "$TESTBIN" > "$CLIENT_LOG" 2>&1
fi
RETCODE=$?
echo "> Done ($RETCODE)."
exit "$RETCODE"
