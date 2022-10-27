#!/usr/bin/env bash

set -eo pipefail

for f in ./src/*.cpp ./FactGenerator/**/*.cpp; do
  clang-format-14 -i "${f}"
done
