#!/usr/bin/env bash

set -eo pipefail

for f in ./*.cpp ./FactGenerator/**/*.cpp; do
  clang-format-10 "${f}" | diff "${f}" -
done

cd test
mypy .
