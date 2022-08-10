#!/bin/bash

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${script_dir}"

INPUTTMP=$(mktemp /tmp/generate-debug-output.XXXXXX)
DECLSTMP=$(mktemp /tmp/generate-debug-output.XXXXXX)

grep -Rho "^\.input [^\(]*" . | sed s/\.input\ //1 | sort >"${INPUTTMP}"
grep -Rh "^\.decl" . | grep -v ' inline$' | grep -o "^\.decl [^\(]*" | sed s/\.decl\ //1 | sort >"${DECLSTMP}"

DUPLICATEDECLS=$(cat "${DECLSTMP}" | uniq -d)
if [ -n "${DUPLICATEDECLS}" ];
then
    echo "Cannot generate debug outputs: duplicate declarations..."
    echo "${DUPLICATEDECLS}"
    rm "${INPUTTMP}"
    rm "${DECLSTMP}"
    exit 1
fi

DUPLICATEINPUTS=$(cat "${INPUTTMP}" | uniq -d)
if [ -n "${DUPLICATEINPUTS}" ];
then
    echo "Cannot generate debug outputs: duplicate input directives..."
    echo "${DUPLICATEINPUTS}"
    rm "${INPUTTMP}"
    rm "${DECLSTMP}"
    exit 2
fi

cat "${DECLSTMP}" | uniq -u | awk '{print ".output " $1 " (compress=true)"}' > export/debug-output-extended.dl
sort "${DECLSTMP}" "${INPUTTMP}" "${INPUTTMP}" | uniq -u | awk '{print ".output " $1 " (compress=true)"}' > export/debug-output.dl

rm "${INPUTTMP}"
rm "${DECLSTMP}"
