#!/usr/bin/env bash

set -xeo pipefail

# Repository
readonly repo=${REPO:?input REPO is required}
# Release number
readonly version=${VERSION:-''}

readonly out=$GITHUB_WORKSPACE
readonly repo_name=${repo#*/}
readonly archive_lib=$out/$repo_name-$version.zip
readonly archive_deb=$out/$repo_name-$version-deb-pkgs.zip
readonly archive_rpm=$out/$repo_name-$version-rpm-pkgs.zip

# Make packages into build/packages
mkdir -p build
cd build
cmake ../install
cmake --build . --target package
ls -R

cd "$GITHUB_WORKSPACE"
mv "build/packages/$repo_name-$version.zip" "$archive_lib"
zip --verbose --junk-paths "$archive_deb" build/packages/*.deb
zip --verbose --junk-paths "$archive_rpm" build/packages/*.rpm

{ echo "archive-lib=$(basename "$archive_lib")";
  echo "archive-deb=$(basename "$archive_deb")";
  echo "archive-rpm=$(basename "$archive_rpm")";
} >> "$GITHUB_OUTPUT"
