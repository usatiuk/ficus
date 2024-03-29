#!/bin/bash
set -euxo pipefail

if [ -z "$OS2_ROOT" ]; then
    echo "$OS2_ROOT" is blank
fi

mkdir -p $OS2_ROOT/toolchain || exit 1

pushd $OS2_ROOT/toolchain

mkdir -p gcc-i686-elf

pushd gcc-i686-elf

if [ ! -d gcc-13.2.0 ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
    tar xvf gcc-13.2.0.tar.xz
    rm gcc-13.2.0.tar.xz
fi

cd gcc-13.2.0
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

patch -p1 -u -i "$SCRIPT_DIR/p1.patch"
patch -p1 -u -i "$SCRIPT_DIR/p2.patch"

cd ..

mkdir -p build
pushd build

export PREFIX="$OS2_ROOT/toolchain/gcc-i686-elf-prefix/"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

if [ ! -f "$OS2_ROOT/toolchain/gcc-i686-elf-prefix/bin/i686-elf-as" ]; then
    echo "binutils not found"
    exit 1
fi

HOMEBREW_LIBS="--with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew"

ADDONS=""

if [ ! -z ${USE_BREW_LIBS+x} ]; then
    ADDONS="$HOMEBREW_LIBS $ADDONS"
fi

../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --with-newlib --without-headers --enable-version-specific-runtime-libs $ADDONS
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

touch -m ../done
