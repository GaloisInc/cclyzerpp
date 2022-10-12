#!/usr/bin/env bash

set -eo pipefail

grep 'cclyzer++' CMakeLists.txt | \
  grep -E -o '[0-9]\.[0-9]\.?[0-9]?'
