#!/usr/bin/env bash
set -e

if [ "$#" -eq 0 ]; then
    echo "Usage: build_local PROJECT_PATH INSTALL_PATH [CMAKE_OPTIONS]"
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
cmake .. "${@:3}" -DCMAKE_BUILD_TYPE=Release --install-prefix "$absolute_install_location"
cmake --build . --config Release