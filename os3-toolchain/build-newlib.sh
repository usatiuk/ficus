#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

export PREFIX="$OS2_ROOT/toolchain/gcc-x86_64-os3-prefix/"
export TARGET=x86_64-os3
export PATH="$PREFIX/bin:$PATH"
cd "$SCRIPT_DIR"
cd newlib
# rm -rf build
mkdir -p build
cd build
../newlib-4.4.0.20231231/configure --enable-newlib-supplied-syscalls --prefix=/usr --target=$TARGET
make -j$(nproc) all
make DESTDIR="$OS2_ROOT/sysroot" install
cp -r "$OS2_ROOT/sysroot/usr"/x86_64-os3/* "$OS2_ROOT/sysroot/usr"
