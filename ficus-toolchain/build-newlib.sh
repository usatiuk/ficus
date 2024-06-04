#!/bin/bash
set -euxo pipefail

if [ -z "$SCRIPT_DIR" ]; then
    echo "SCRIPT_DIR" is blank! Run this via build-all
fi

cd "$SCRIPT_DIR"

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    exit 1
fi

case "$1" in
  "conf")
    ;;
  "noconf")
    ;;
  *)
    echo "Unknown option"
    exit 1
    ;;
esac

cd newlib

if [[ "$1" = "conf" ]]
then
  rm -rf build
fi

mkdir -p build
cd build

MAKE_LAUNCHER=

if [[ "$1" = "conf" ]]
then
  if [[ -z ${CI+x} ]]
  then
    MAKE_LAUNCHER="compiledb -o $FICUS_ROOT/ficus-toolchain/newlib/newlib-4.4.0.20231231/compile_commands.json"
  fi
  ../newlib-4.4.0.20231231/configure --enable-newlib-supplied-syscalls --prefix=/usr --target=$TARGET
fi

$MAKE_LAUNCHER make -j$BUILD_PARALLEL all
make DESTDIR="$FICUS_ROOT/sysroot" install
cp -r "$FICUS_ROOT/sysroot/usr"/x86_64-ficus/* "$FICUS_ROOT/sysroot/usr"
