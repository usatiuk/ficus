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

mkdir -p "$FICUS_ROOT/sysroot"

cd binutils-x86_64-ficus
mkdir -p build
cd build
../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$FICUS_ROOT/sysroot" --disable-nls --disable-werror
make -j$(nproc)
make install

cd ../../

cd gcc-x86_64-ficus
rm -rf build
mkdir -p build
cd build
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$FICUS_ROOT/sysroot" --disable-nls --enable-languages=c,c++ --with-newlib --without-headers --enable-version-specific-runtime-libs --with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
cd ../../

"$SCRIPT_DIR"/build-newlib.sh

cd gcc-x86_64-ficus
rm -rf build
mkdir -p build
cd build
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$FICUS_ROOT/sysroot" --disable-nls \
    --enable-languages=c,c++ --with-newlib --disable-libstdcxx-threads \
    --enable-version-specific-runtime-libs --with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make -j$(nproc) all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
