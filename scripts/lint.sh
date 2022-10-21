#!/usr/bin/env bash

set -eo pipefail

for f in ./src/*.cpp ./FactGenerator/**/*.cpp; do
  "clang-format-${CLANG_VERSION}" "${f}" | diff "${f}" -
done

if ! [[ -f build/compile_commands.json ]]; then
  printf "Run cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G Ninja -S . -B build.\n"
  exit 1
fi

cmake --build build -j "$(nproc)" --target factgen-tidy

cd test
mypy .
mypy ../scripts/*.py
