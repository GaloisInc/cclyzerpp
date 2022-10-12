#!/usr/bin/env bash

# Package cclyzer++. See doc/dev.rst for more info.

set -euo pipefail

if [[ "${1:-undef}" == undef ]]; then
  printf "%s\n" "Usage: ./pkg.sh VERSION"
  exit 1
fi

# See https://stackoverflow.com/a/246128/3561275
script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "${script_dir}"
unset script_dir

# On Debian package names:
#
# https://www.debian.org/doc/manuals/debian-faq/pkg-basics.en.html#pkgname

# See also ./.fpm
fpm \
  -t deb \
  -p "cclyzer++_${1}-${2:-1}_amd64.deb" \
  --version "${1}"
