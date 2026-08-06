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

resolve_interface_ip() {
    case "${RUNNER_OS:-$(uname -s)}" in
        Linux)
            ip route get 1.1.1.1 |
                awk '{ for (i = 1; i <= NF; i++) if ($i == "src") { print $(i + 1); exit } }' |
                select_private_ipv4
            ;;
        macOS|Darwin)
            INTERFACE=$(route -n get default | awk '/interface:/{print $2; exit}')
            ipconfig getifaddr "$INTERFACE" | select_private_ipv4
            ;;
        Windows|MINGW*|MSYS*)
            powershell.exe -NoProfile -Command '
                $ErrorActionPreference = "Stop"
                try {
                    $route = Get-NetRoute `
                        -AddressFamily IPv4 `
                        -DestinationPrefix "0.0.0.0/0" `
                        -PolicyStore ActiveStore |
                        Sort-Object @{Expression = { $_.RouteMetric + $_.InterfaceMetric }} |
                        Select-Object -First 1

                    if ($null -eq $route) { exit 1 }

                    $ip = Get-NetIPAddress `
                        -AddressFamily IPv4 `
                        -InterfaceIndex $route.InterfaceIndex `
                        -AddressState Preferred |
                        Where-Object {
                            $_.IPAddress -match "^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[0-1])\.)"
                        } |
                        Select-Object -First 1 -ExpandProperty IPAddress

                    if ($ip) {
                        $ip
                        exit 0
                    }

                    exit 1
                } catch {
                    exit 1
                }
            ' |
                tr -d '\r'
            ;;
        *)
            return 1
            ;;
    esac
}

select_private_ipv4() {
    while IFS= read -r candidate; do
        if is_private_ipv4 "$candidate"; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

is_valid_ipv4() {
    printf '%s\n' "$1" | awk -F. '
        NF != 4 { exit 1 }
        {
            for (i = 1; i <= 4; i++) {
                if ($i !~ /^[0-9]+$/ || $i < 0 || $i > 255) exit 1
            }
        }
    '
}

is_private_ipv4() {
    is_valid_ipv4 "$1" || return 1

    case "$1" in
        10.*|192.168.*|172.16.*|172.17.*|172.18.*|172.19.*|172.2[0-9].*|172.3[0-1].*)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

if [ -z "${ZENOH_TEST_ROUTER_LISTENER:-}" ]; then
    INTERFACE_IP=$(resolve_interface_ip)
    if ! is_private_ipv4 "$INTERFACE_IP"; then
        echo "ERROR: could not resolve a private IPv4 address for the test interface." >&2
        echo "Resolved address: ${INTERFACE_IP:-<empty>}" >&2
        echo "Set ZENOH_TEST_ROUTER_LISTENER to a concrete interface address before running ctest, or connect this machine to a private network." >&2
        echo "Refusing to fall back to tcp/0.0.0.0:7447: that would expose the unauthenticated test router on every network interface." >&2
        exit 1
    fi
    ZENOH_TEST_ROUTER_LISTENER="tcp/$INTERFACE_IP:7447"
    export ZENOH_TEST_ROUTER_LISTENER
fi

# get vinary name without extension
TEST_NAME_WE=$(basename -- "$TESTBIN")
TEST_NAME_WE="${TEST_NAME_WE%.*}"

cd "$TESTDIR"|| exit

echo "------------------ Running test $TESTBIN -------------------"

cleanup() {
    if [ -n "${ZPID:-}" ]; then
        kill -9 "$ZPID" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

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

LOCATORS="$ZENOH_TEST_ROUTER_LISTENER"
for LOCATOR in $(echo "$LOCATORS" | xargs); do
    sleep 1

    echo "> Running zenohd ... $LOCATOR"
    RUST_LOG=debug ./zenohd --plugin-search-dir "$TESTDIR/zenoh-git/target/debug" -l "$LOCATOR" > zenohd."$TEST_NAME_WE".log 2>&1 &
    ZPID=$!

    sleep 5

    echo "> Running $TESTBIN ..."
    "$TESTBIN" "$LOCATOR" > client."$TEST_NAME_WE".log 2>&1
    RETCODE=$?

    echo "> Logs of $TESTBIN ..."
    cat client."$TEST_NAME_WE".log

    echo "> Stopping zenohd ..."
    kill -9 "$ZPID"

    sleep 1

    echo "> Logs of zenohd ..."
    cat zenohd."$TEST_NAME_WE".log

    [ "$RETCODE" -lt 0 ] && exit "$RETCODE"
done

echo "> Done ($RETCODE)."
exit "$RETCODE"
