#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

export PREFIX="$FICUS_ROOT/toolchain/gcc-x86_64-ficus-prefix/"
export TARGET=x86_64-ficus
export PATH="$PREFIX/bin:$PATH"
cd "$SCRIPT_DIR"
cd newlib
# rm -rf build
mkdir -p build
cd build
../newlib-4.4.0.20231231/configure --enable-newlib-supplied-syscalls --prefix=/usr --target=$TARGET
make -j$(nproc) all
make DESTDIR="$FICUS_ROOT/sysroot" install
cp -r "$FICUS_ROOT/sysroot/usr"/x86_64-ficus/* "$FICUS_ROOT/sysroot/usr"
