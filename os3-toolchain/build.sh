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

mkdir -p "$OS2_ROOT/sysroot"

cd binutils-x86_64-os3
mkdir -p build
cd build
../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$OS2_ROOT/sysroot" --disable-nls --disable-werror
make -j$(nproc)
make install

cd ../../

cd gcc-x86_64-os3
rm -rf build
mkdir -p build
cd build
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$OS2_ROOT/sysroot" --disable-nls --enable-languages=c,c++ --with-newlib --without-headers --enable-version-specific-runtime-libs --with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

cd ../../
cd newlib
mkdir build
cd build
../newlib-4.4.0.20231231/configure --prefix=/usr --target=$TARGET
make -j$(nproc) all
make DESTDIR="$OS2_ROOT/sysroot" install
cp -r "$OS2_ROOT/sysroot/usr"/x86_64-os3/* "$OS2_ROOT/sysroot/usr"
cd ../../

cd gcc-x86_64-os3
rm -rf build
mkdir -p build
cd build
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$OS2_ROOT/sysroot" --disable-nls --enable-languages=c,c++ --with-newlib --enable-version-specific-runtime-libs --with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
