#!/bin/bash
set -euxo pipefail

if [ -z "$FICUS_ROOT" ]; then
  echo "$FICUS_ROOT" is blank
fi

mkdir -p $FICUS_ROOT/toolchain || exit 1

cd $FICUS_ROOT/toolchain

cd gcc-x86_64-elf || exit 1

if [ ! -d gcc-13.2.0 ]; then
  echo "GCC Sources not found!"
  exit 1
fi

if [ ! -f "$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/bin/x86_64-elf-as" ]; then
  echo "binutils not found"
  exit 1
fi

if [ ! -f "$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/bin/x86_64-elf-gcc" ]; then
  echo "gcc not found"
  exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

patch -p0 -u -i "$SCRIPT_DIR/gcc-libstdcpp.patch"

export PREFIX="$FICUS_ROOT/toolchain/gcc-x86_64-elf-prefix/"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Note that you might have to go into the configure script and remove tests for fcntl and maybe other errors
mkdir -p libstdcppbuild
cd libstdcppbuild
../gcc-13.2.0/libstdc++-v3/configure \
  --host=$TARGET \
  --prefix="$PREFIX" \
  --disable-nls \
  --with-newlib \
  --without-headers \
  --disable-libstdcxx-threads \
  --disable-hosted-libstdcxx \
  --enable-version-specific-runtime-libs

make -j$(nproc) CFLAGS_FOR_TARGET='-g -O2 -mcmodel=large -mno-red-zone'
make install

touch -m ../donelibstdcpp
