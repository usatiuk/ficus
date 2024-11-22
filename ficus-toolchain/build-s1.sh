#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
    exit 1
fi

if [ -z "$PREFIX" ]; then
    echo "PREFIX" is blank
    exit 1
fi

if [ -z "$SCRIPT_DIR" ]; then
    echo "SCRIPT_DIR" is blank
    exit 1
fi

cd "$SCRIPT_DIR"

mkdir -p "$FICUS_ROOT/sysroot"

cd binutils-x86_64-ficus
mkdir -p build
cd build
../source/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$PREFIX/../../sysroot" --disable-nls --disable-werror
make -j$(nproc)
make install

cd ../../

cd gcc-x86_64-ficus
rm -rf build
mkdir -p build
cd build

pushd ../source/
./contrib/download_prerequisites
popd

../source/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$PREFIX/../../sysroot" --disable-nls --enable-languages=c,c++ --with-newlib --without-headers --disable-fixincludes --enable-version-specific-runtime-libs
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
find "$PREFIX" -exec strip {} \;
cd ../../

