#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi

mkdir -p $OS2_ROOT/toolchain || exit 1

pushd $OS2_ROOT/toolchain

mkdir -p limine

pushd limine

mkdir -p prefix

if [ ! -d "limine-5.20230830.0" ]; then
    wget https://github.com/limine-bootloader/limine/releases/download/v5.20230830.0/limine-5.20230830.0.tar.xz
    tar xvf limine-5.20230830.0.tar.xz
    rm limine-5.20230830.0.tar.xz
fi

mkdir -p build

pushd build

if [ ! -f "$OS2_ROOT/toolchain/gcc-i686-elf-prefix/bin/i686-elf-gcc" ]; then
    echo "binutils not found"
    exit 1
fi

export PATH="$OS2_ROOT/toolchain/gcc-i686-elf-prefix/bin":"$PATH"
export PATH="$OS2_ROOT/toolchain/gcc-x86_64-elf-prefix/bin":"$PATH"

export PREFIX="$OS2_ROOT/toolchain/limine/prefix"
export PATH="$PREFIX/bin:$PATH"

# fix for old make
grep -rl "define DEFAULT_VAR =" ../limine-5.20230830.0 | xargs sed -i.bak -e 's/define DEFAULT_VAR =/define DEFAULT_VAR/g'

../limine-5.20230830.0/configure --disable-werror --enable-bios-cd --enable-bios --enable-uefi-ia32 --enable-uefi-x86-64 --enable-uefi-cd \
    --prefix="$PREFIX"

make -j$(nproc) install

touch -m ../done
