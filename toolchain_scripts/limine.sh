#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
fi

mkdir -p $FICUS_ROOT/toolchain || exit 1

pushd $FICUS_ROOT/toolchain

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

if [ ! -f "$FICUS_ROOT/toolchain/gcc-i686-elf-prefix/bin/i686-elf-gcc" ]; then
    echo "binutils not found"
    exit 1
fi

export PATH="$FICUS_ROOT/toolchain/gcc-i686-elf-prefix/bin":"$PATH"
export PATH="$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/bin":"$PATH"

export PREFIX="$FICUS_ROOT/toolchain/limine/prefix"
export PATH="$PREFIX/bin:$PATH"

# fix for old make
grep -rl "define DEFAULT_VAR =" ../limine-5.20230830.0 | xargs sed -i.bak -e 's/define DEFAULT_VAR =/define DEFAULT_VAR/g'

../limine-5.20230830.0/configure --disable-werror --enable-bios-cd --enable-bios --enable-uefi-ia32 --enable-uefi-x86-64 --enable-uefi-cd \
    --prefix="$PREFIX"

make -j$(nproc) install

touch -m ../done
