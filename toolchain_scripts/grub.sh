#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi

mkdir -p $OS2_ROOT/toolchain || exit 1

pushd $OS2_ROOT/toolchain

mkdir -p grub

pushd grub

mkdir -p prefix

if [ ! -d "grub-2.06" ]; then
    wget https://ftp.gnu.org/gnu/grub/grub-2.06.tar.xz
    tar xvf grub-2.06.tar.xz
    rm grub-2.06.tar.xz
fi

mkdir -p build

pushd build

if [ ! -f "$OS2_ROOT/toolchain/gcc-i686-elf-prefix/bin/i686-elf-gcc" ]; then
    echo "binutils not found"
    exit 1
fi

export PATH="$OS2_ROOT/toolchain/gcc-i686-elf-prefix/bin":"$PATH"

export PREFIX="$OS2_ROOT/toolchain/grub/prefix"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

../grub-2.06/configure --disable-werror TARGET_CC=i686-elf-gcc TARGET_AS=i686-elf-as TARGET_OBJCOPY=i686-elf-objcopy \
    TARGET_STRIP=i686-elf-strip TARGET_NM=i686-elf-nm TARGET_RANLIB=i686-elf-ranlib --target="$TARGET" \
    --prefix="$PREFIX"

make -j$(nproc)
make install

touch -m ../done
