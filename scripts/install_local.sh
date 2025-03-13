#!/usr/bin/env bash
set -e

if [ "$#" -eq 0 ]; then
    echo "Usage: install_local PROJECT_PATH INSTALL_PATH [CONFIG] [CMAKE_ARGS]"
    exit
fi

CONFIG="Release"

if [ "$#" -ge 3 ]; then
    CONFIG=$3
fi


current_dir=$PWD
project_folder=$1
mkdir -p $2
absolute_install_location=$(cd $2; pwd)

echo $install_location
cd "$project_folder"
rm -rf ./build
mkdir ./build
cd ./build
cmake .. "${@:4}" --install-prefix "$absolute_install_location"
cmake --build . --target install --config $CONFIG
