#!/usr/bin/env bash

set -xeo pipefail

# Release number
readonly version=${VERSION:?input VERSION is required}
# Branch
readonly branch=${BRANCH:?input BRANCH is required}
# Branch of zenoh-c
readonly zenoh_c_branch=${ZENOH_C_BRANCH:-''}
# Branch of zenoh-pico
readonly zenoh_pico_branch=${ZENOH_PICO_BRANCH:-''}
# Git actor name
readonly git_user_name=${GIT_USER_NAME:?input GIT_USER_NAME is required}
# Git actor email
readonly git_user_email=${GIT_USER_EMAIL:?input GIT_USER_EMAIL is required}

export GIT_AUTHOR_NAME=$git_user_name
export GIT_AUTHOR_EMAIL=$git_user_email
export GIT_COMMITTER_NAME=$git_user_name
export GIT_COMMITTER_EMAIL=$git_user_email

# Bump CMake project version
printf '%s' "$version" > version.txt

# Set branches
printf '%s' "$branch" > zenoh-cpp-branch.txt
if [[ $zenoh_c_branch != '' ]]; then
  printf '%s' "$zenoh_c_branch" > zenoh-c-branch.txt
fi
if [[ $zenoh_pico_branch != '' ]]; then
  printf '%s' "$zenoh_pico_branch" > zenoh-pico-branch.txt
fi

git commit version.txt -m "chore: Bump version to $version"
git tag --force "$version" -m "v$version"
git log -10
git show-ref --tags
git push --force origin
git push --force origin "$version"
