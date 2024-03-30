#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
fi

mkdir -p $FICUS_ROOT/toolchain || exit 1

pushd $FICUS_ROOT/toolchain

mkdir -p binutils-i686-elf

pushd binutils-i686-elf

if [ ! -d "binutils-2.41" ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
    tar xvf binutils-2.41.tar.xz
    rm binutils-2.41.tar.xz
fi

mkdir -p build

pushd build

export PREFIX="$FICUS_ROOT/toolchain/gcc-i686-elf-prefix/"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

make -j$(nproc)
make install

touch -m ../done
