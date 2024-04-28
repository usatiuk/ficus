#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
fi
export SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

export PREFIX="$FICUS_ROOT/toolchain/gcc-x86_64-ficus-prefix/"
export TARGET=x86_64-ficus
export PATH="$PREFIX/bin:$PATH"

cd "$SCRIPT_DIR"

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit 1
fi

if [ -z "${BUILD_PARALLEL}" ]
then
  export BUILD_PARALLEL=$(nproc)
fi
if [ -z "${BUILD_PARALLEL_GCC}" ]
then
  export BUILD_PARALLEL_GCC=$BUILD_PARALLEL
fi

if [[ "$1" != "s2only" ]]
then
  "$SCRIPT_DIR"/build-s1.sh
  cd "$SCRIPT_DIR"
fi

if [[ "$1" = "s1only" ]]
then exit 0
fi

mkdir -p "$FICUS_ROOT/sysroot"

"$SCRIPT_DIR"/build-newlib.sh
cd "$SCRIPT_DIR"
"$SCRIPT_DIR"/build-s2.sh
cd "$SCRIPT_DIR"
