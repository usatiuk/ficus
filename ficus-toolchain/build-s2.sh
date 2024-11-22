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

cd gcc-x86_64-ficus
rm -rf build
mkdir -p build
cd build

pushd ../source/
./contrib/download_prerequisites
popd

../source/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$PREFIX/../../sysroot" --disable-nls \
    --enable-languages=c,c++ --with-newlib --disable-fixincludes --disable-libstdcxx-threads \
    --enable-version-specific-runtime-libs
make -j$BUILD_PARALLEL all-gcc
make -j$BUILD_PARALLEL all-target-libgcc
make -j$BUILD_PARALLEL all-target-libstdc++-v3
make install-gcc
find "$PREFIX" -exec strip {} \;
make install-target-libgcc
make install-target-libstdc++-v3
