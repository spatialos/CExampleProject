#!/usr/bin/env bash

set -e -u -x -o pipefail

cd "$(dirname "$0")/.."

PLATFORM=""
if [[ "$(uname -s)" == "Linux" ]]; then
  PLATFORM="linux"
elif [[ "$(uname -s)" == "Darwin" ]]; then
  PLATFORM="macos"
else
  PLATFORM="windows"
fi
spatial build --target $PLATFORM --log_level=debug
