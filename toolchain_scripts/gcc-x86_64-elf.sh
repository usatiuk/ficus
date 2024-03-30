#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
    echo "$FICUS_ROOT" is blank
fi

mkdir -p $FICUS_ROOT/toolchain || exit 1

pushd $FICUS_ROOT/toolchain

mkdir -p gcc-x86_64-elf

pushd gcc-x86_64-elf

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

export PREFIX="$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

if [ ! -f "$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/bin/x86_64-elf-as" ]; then
    echo "binutils not found"
    exit 1
fi

HOMEBREW_LIBS="--with-gmp=/opt/homebrew --with-mpc=/opt/homebrew --with-mpfr=/opt/homebrew"

ADDONS=""

if [ ! -z ${USE_BREW_LIBS+x} ]; then
    ADDONS="$HOMEBREW_LIBS $ADDONS"
fi

cat <<EOF >../gcc-13.2.0/gcc/config/i386/t-x86_64-elf
# Add libgcc multilib variant without red-zone requirement
 
MULTILIB_OPTIONS += mno-red-zone
MULTILIB_DIRNAMES += no-red-zone
EOF

sed -i.bak 's/x86_64-\*-elf\*)/x86_64-\*-elf\*)\n\ttmake_file="\${tmake_file} i386\/t-x86_64-elf"/g' "../gcc-13.2.0/gcc/config.gcc"

../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --with-newlib --without-headers --enable-version-specific-runtime-libs $ADDONS
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

touch -m ../done
