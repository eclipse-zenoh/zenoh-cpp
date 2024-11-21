#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "$#" -eq 0 ]; then
    echo "Usage: install_from_git INSTALL_PATH [BUILD_WITH_UNSTABLE_API] [BUILD_WITH_SHARED_MEMORY] [BUILD_PICO]"
    exit
fi

USE_UNSTABLE="TRUE"
USE_SHARED_MEMORY="TRUE"
USE_UNSTABLE_PICO="0"
BUILD_PICO="ON"

if [ "$#" -ge 2 ]; then
    USE_UNSTABLE=$2
fi

if [ "$#" -ge 3 ]; then
    USE_SHARED_MEMORY=$3
fi

if [ "$USE_UNSTABLE" == "TRUE" ]; then
    USE_UNSTABLE_PICO="1"
fi

if [ "$#" -ge 4 ]; then
    BUILD_PICO=$4
fi


git submodule init
git submodule update

mkdir -p $1
absolute_install_location=$(cd $1; pwd)
#build zenoh-c
bash $SCRIPT_DIR/install_local.sh $SCRIPT_DIR/../zenoh-c $absolute_install_location -DZENOHC_BUILD_WITH_UNSTABLE_API=$USE_UNSTABLE -DZENOHC_BUILD_WITH_SHARED_MEMORY=$USE_SHARED_MEMORY
if [ "$BUILD_PICO" == "ON" ]; then
    #build zenoh-pico
    bash $SCRIPT_DIR/install_local.sh $SCRIPT_DIR/../zenoh-pico $absolute_install_location -DZ_FEATURE_UNSTABLE_API=$USE_UNSTABLE_PICO -DZ_FEATURE_LIVELINESS=$USE_UNSTABLE_PICO
fi

rm -rf ./build
mkdir ./build
cd ./build
cmake .. -DCMAKE_BUILD_TYPE=Release -DZENOHCXX_ZENOHPICO=$BUILD_PICO  --install-prefix "$absolute_install_location"
cmake --build . --target install --config Release