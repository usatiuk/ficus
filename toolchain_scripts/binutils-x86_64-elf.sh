#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi

mkdir -p $OS2_ROOT/toolchain || exit 1

pushd $OS2_ROOT/toolchain

mkdir -p binutils-x86_64-elf

pushd binutils-x86_64-elf

if [ ! -d "binutils-2.41" ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
    tar xvf binutils-2.41.tar.xz
    rm binutils-2.41.tar.xz
fi

mkdir -p build

pushd build

export PREFIX="$OS2_ROOT/toolchain/gcc-x86_64-elf-prefix/"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

make -j$(nproc)
make install

touch -m ../done
