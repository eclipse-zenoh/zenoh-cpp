#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$#" -eq 0 ]; then
    echo "Usage: install_from_local INSTALL_PATH"
    exit
fi

mkdir -p $1
absolute_install_location=$(cd $1; pwd)
#build zenoh-c
bash $SCRIPT_DIR/install_local.sh $SCRIPT_DIR/../../zenoh-c $absolute_install_location -DZENOHC_BUILD_WITH_UNSTABLE_API=TRUE -DZENOHC_BUILD_WITH_SHARED_MEMORY=TRUE
#build zenoh-pico
bash $SCRIPT_DIR/install_local.sh $SCRIPT_DIR/../../zenoh-pico $absolute_install_location -DZ_FEATURE_UNSTABLE_API=1

rm -rf ./build
mkdir ./build
cd ./build
cmake .. -DCMAKE_BUILD_TYPE=Release -DZENOHCXX_ZENOHPICO=ON --install-prefix "$absolute_install_location"
cmake --build . --target install --config Release