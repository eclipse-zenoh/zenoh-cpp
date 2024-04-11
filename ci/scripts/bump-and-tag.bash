#!/usr/bin/env bash

set -eo pipefail

# Repository
readonly repo=${REPO:?input REPO is required}
# Release number
readonly version=${VERSION:-''}
# Release branch
readonly branch=${BRANCH:?input BRANCH is required}
# GitHub token
readonly github_token=${GITHUB_TOKEN:?input GITHUB_TOKEN is required}
# Git actor name
readonly git_user_name=${GIT_USER_NAME:?input GIT_USER_NAME is required}
# Git actor email
readonly git_user_email=${GIT_USER_EMAIL:?input GIT_USER_EMAIL is required}

export GIT_AUTHOR_NAME=$git_user_name
export GIT_AUTHOR_EMAIL=$git_user_email
export GIT_COMMITTER_NAME=$git_user_name
export GIT_COMMITTER_EMAIL=$git_user_email

git clone --recursive --single-branch --branch "$branch" "https://github.com/$repo"

# Bump CMake project version
if [[ "$version" == '' ]]; then
  # If no version has been specified, infer it using git-describe
  printf '%s' "$(git describe)" > version.txt
else
  printf '%s' "$version" > version.txt
fi

git commit version.txt -m "chore: Bump version to $version"
git tag "$version" -m "v$version"
git log -10
git show-ref --tags
git push "https://$github_token@github.com/$repo" "$branch" "$version"
