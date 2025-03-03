#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$#" -eq 0 ]; then
    echo "Usage: build_standalone_examples INSTALL_PATH [BUILD_PICO]"
    exit
fi
BUILD_PICO="ON"
if [ "$#" -ge 2 ]; then
    BUILD_PICO=$2
fi

absolute_install_location=$(cd $1; pwd)

bash $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/zenohc $absolute_install_location

if [ "$BUILD_PICO" == "ON" ] || [ "$BUILD_PICO" == "TRUE" ] || [ "$BUILD_PICO" == "1" ]; then
    #build examples requiring zenoh-pico
    bash $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/universal $absolute_install_location
    bash $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/zenohpico $absolute_install_location
fi

