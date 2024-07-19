#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$#" -eq 0 ]; then
    echo "Usage: build_standalone_examples INSTALL_PATH"
    exit
fi

absolute_install_location=$(cd $1; pwd)

sh $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/universal $absolute_install_location
sh $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/zenohc $absolute_install_location
sh $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../examples/simple/zenohpico $absolute_install_location