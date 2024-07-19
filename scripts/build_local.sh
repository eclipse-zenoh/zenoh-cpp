#!/usr/bin/env bash
set -e

if [ "$#" -eq 0 ]; then
    echo "Usage: build_local [CMAKE_OPTIONS] PROJECT_PATH INSTALL_PATH"
    exit
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
echo "${@:3}"
cmake .. "${@:3}" --install-prefix "$absolute_install_location"
make
make install