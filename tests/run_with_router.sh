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

# get vinary name without extension
TEST_NAME_WE=$(basename -- "$TESTBIN")
TEST_NAME_WE="${TEST_NAME_WE%.*}"

cd "$TESTDIR"|| exit

echo "------------------ Running test $TESTBIN -------------------"

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

LOCATORS="tcp/127.0.0.1:7447"
for LOCATOR in $(echo "$LOCATORS" | xargs); do
    sleep 1

    echo "> Running zenohd ... $LOCATOR"
    RUST_LOG=debug ./zenohd --plugin-search-dir "$TESTDIR/zenoh-git/target/debug" -l "$LOCATOR" > zenohd."$TEST_NAME_WE".log 2>&1 &
    ZPID=$!

    sleep 5

    echo "> Running $TESTBIN ..."
    "$TESTBIN" "$LOCATOR"
    RETCODE=$?

    echo "> Stopping zenohd ..."
    kill -9 "$ZPID"

    sleep 1

    echo "> Logs of zenohd ..."
    cat zenohd."$TEST_NAME_WE".log

    [ "$RETCODE" -lt 0 ] && exit "$RETCODE"
done

echo "> Done ($RETCODE)."
exit "$RETCODE"
