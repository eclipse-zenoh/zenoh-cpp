#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$#" -eq 0 ]; then
    echo "Usage: install_from_git INSTALL_PATH"
    exit
fi

git submodule init
git submodule update

mkdir -p $1
absolute_install_location=$(cd $1; pwd)
#build zenoh-c
sh $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../zenoh-c $absolute_install_location -DZENOHC_BUILD_WITH_UNSTABLE_API=TRUE -DZENOHC_BUILD_WITH_SHARED_MEMORY=TRUE
#build zenoh-pico
sh $SCRIPT_DIR/build_local.sh $SCRIPT_DIR/../zenoh-pico $absolute_install_location

rm -rf ./build
mkdir ./build
cd ./build
cmake .. --install-prefix "$absolute_install_location"
make
make install